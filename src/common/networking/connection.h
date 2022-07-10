#pragma once
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ssl.hpp>


#include "message.h"
#include <cstdint>
#include <memory>
#include "config/config.h"
#include <utility/asyncQueue.h>
#include <utility/asyncData.h>
#include <utility/serializedData.h>
#include <ecs/ecs.h>
#include <shared_mutex>
#include "request.h"

namespace net
{

	typedef asio::ip::tcp::socket tcp_socket;
	typedef asio::ssl::stream<tcp_socket> ssl_socket;
	class Request;

	class Connection
	{
	protected:
		std::mutex _oLock;
		std::deque<std::shared_ptr<OMessage>> _obuffer;
		AsyncQueue<std::shared_ptr<IMessage>> _ibuffer;
		std::shared_mutex _streamLock;
		std::unordered_map<uint32_t, std::function<void(ISerializedData& data)>> _streamListeners;
		uint32_t _reqIDCounter = 1000;
		std::shared_mutex _responseLock;
		std::unordered_map<uint32_t, AsyncData<ISerializedData>> _responseListeners;
		std::shared_ptr<IMessage> _tempIn;
		std::shared_ptr<bool> _exists;

		std::vector<std::function<void()>> _onDisconnect;

	public:
		Connection();
		virtual ~Connection();
		virtual void disconnect() = 0;
		virtual bool connected() = 0;

		virtual void send(std::shared_ptr<OMessage> msg) = 0;
		void sendStreamData(uint32_t id, OSerializedData&& sData);
		void sendStreamEnd(uint32_t id);
		void addStreamListener(uint32_t id, std::function<void(ISerializedData& data)> callback);
		void eraseStreamListener(uint32_t id);
		AsyncData<ISerializedData> sendRequest(Request&& req);
		AsyncData<ISerializedData> sendRequest(Request& req);
		void onDisconnect(std::function<void()> f);
		bool messageAvailable();
		std::shared_ptr<IMessage> popMessage();

	};

	template<class socket_t>
	class ConnectionBase : public Connection
	{

	protected:
		socket_t _socket;
		std::string _address;
		void async_readHeader()
		{
			std::shared_ptr<bool> exists = _exists;
			_tempIn = std::make_shared<IMessage>();
			asio::async_read(_socket, asio::buffer(&_tempIn->header, sizeof(MessageHeader)) ,[this, exists](std::error_code ec, std::size_t length) {
				if (!ec && *exists)
				{
					if(_tempIn->header.size > 0)
						async_readBody();
					else
					{
						_ibuffer.push_back(_tempIn);
						async_readHeader();
					}
				}
				else
				{
					if(*exists)
					{
						Runtime::error("[" + _address + "] Header Parse Fail: " + ec.message());
						disconnect();
					}
					else
						Runtime::warn("Socket destroyed");
				}

			});
		}
		void async_readBody()
		{
			std::shared_ptr<bool> exists = _exists;
			_tempIn->body.data.resize(_tempIn->header.size);
			asio::async_read(_socket, asio::buffer(_tempIn->body.data.data(), _tempIn->body.size()),  [this, exists](std::error_code ec, std::size_t length) {
				if (!ec && *exists)
				{
					switch(_tempIn->header.type)
					{
						case MessageType::response:
						{
							uint32_t id;
							_tempIn->body >> id;

							AsyncData<ISerializedData> data;
							{
								std::scoped_lock l(_responseLock);
								if(!_responseListeners.count(id)){
									Runtime::error("Unknown response received: " + std::to_string(id));
									break;
								}
								data = std::move(_responseListeners[id]); // We can't call this from inside the lock, since callback can also request assets, and that also requires a lock
								_responseListeners.erase(id);
							}

							ISerializedData sData;
							_tempIn->body >> sData;
							data.setData(sData);
							break;
						}
						case MessageType::streamData:
						{
							uint32_t id;
							_tempIn->body >> id;

							std::shared_ptr<IMessage> message = _tempIn;
							_streamLock.lock_shared();
							if (_streamListeners.count(id))
							{

								auto f = _streamListeners[id];
								ThreadPool::enqueue([this, id, message, f]
			                    {
				                    ISerializedData data;
				                    message->body >> data;
				                    f(data);
			                    });
							}
							else
								Runtime::error("Unknown stream id (receiving data): " + std::to_string(id));

							_streamLock.unlock_shared();
							break;
						}
						case MessageType::endStream:
						{
							uint32_t id;
							_tempIn->body >> id;
							std::cout << "ending stream: " << id << std::endl;
							std::shared_ptr<IMessage> message = _tempIn;
							_streamLock.lock();
							if (_streamListeners.count(id))
							{

								_streamListeners.erase(id);
							} else
								Runtime::error("Unknown stream id (attempting to end): " + std::to_string(id));
							_streamLock.unlock();
							break;
						}
						default:
							_ibuffer.push_back(_tempIn);
							break;
					}
					async_readHeader();
				}
				else
				{
					if(*exists)
					{
						Runtime::error("[" + _address + "] Read Message Body Fail: " + ec.message());
						disconnect();
					}
					else
						Runtime::log("Socket destroyed");
				}
			});
		}
		void async_writeHeader()
		{
			std::shared_ptr<bool> exists = _exists;
			asio::async_write(_socket, asio::buffer(&_obuffer.front()->header, sizeof(MessageHeader)), [this, exists](std::error_code ec, std::size_t length) {
				if(!*exists)
					return;
				if (!ec)
				{
					async_writeBody();
				}
				else
				{
					Runtime::error("[" + _address + "] Write header fail: " + ec.message());
					disconnect();
				}
			});
		}
		void async_writeBody()
		{
			std::shared_ptr<bool> exists = _exists;
			asio::async_write(_socket, asio::buffer(_obuffer.front()->body.data.data(), _obuffer.front()->header.size), [this, exists](std::error_code ec, std::size_t length) {
				if(!*exists)
					return;
				if (!ec)
				{
					std::scoped_lock lock(_oLock);
					_obuffer.pop_front();
					if(!_obuffer.empty())
						async_writeHeader();
				}
				else
				{
					Runtime::error("[" + _address + "] Write body fail: " + ec.message());
					disconnect();
				}
			});
		}
	public:
		ConnectionBase(socket_t&& socket) : _socket(std::move(socket))
		{
		}
		~ConnectionBase()
		{
		}
		void send(std::shared_ptr<OMessage> msg) override
		{
			assert(msg->body.size() <= 65535); //unsigned int 16 max value
			msg->header.size = msg->body.size();

			std::shared_ptr<bool> exists = _exists;
			asio::post(_socket.get_executor(), [this, exists, msg]()
			{
				if (!*exists)
					return;
				std::scoped_lock lock(_oLock);
				bool sending = !_obuffer.empty();
				_obuffer.push_back(msg);

				if (!sending)
					async_writeHeader();
			});
		}
		void disconnect() override
		{
			std::shared_ptr<bool> exists = _exists;
			asio::post(_socket.get_executor(), [this, exists]{
				if(*exists && connected())
				{
					_socket.lowest_layer().close();
					for(auto& f: _onDisconnect)
						f();
					_responseLock.lock();
					for(auto& r: _responseListeners)
						r.second.setError("Socket Disconnect");
					_responseLock.unlock();
				}
			});
		}
		bool connected() override
		{
			return _socket.lowest_layer().is_open() && _exists;
		}

	};

	template<class socket_t>
	class ServerConnection : public ConnectionBase<socket_t>
	{
	public:
		ServerConnection(socket_t&& socket) : ConnectionBase<socket_t>(std::move(socket))
		{
			connectToClient();
		}
		void connectToClient();
	};



	template<class socket_t>
	class ClientConnection : public ConnectionBase<socket_t>
	{

	public:
		ClientConnection(socket_t&& socket) : ConnectionBase<socket_t>(std::move(socket))
		{
		}
		void connectToServer(const asio::ip::tcp::resolver::results_type& endpoints, std::function<void()> onConnect, std::function<void()> onFail);
	};



	struct NewConnectionComponent : public NativeComponent <NewConnectionComponent>
	{
		REGISTER_MEMBERS_0("New Connection");
	};

	struct ConnectionComponent : public NativeComponent<ConnectionComponent>
	{
		REGISTER_MEMBERS_2("Connection", id, connection);
		ConnectionID id;
		std::shared_ptr<Connection> connection;
		
	};
}