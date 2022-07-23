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

namespace net
{

	typedef asio::ip::tcp::socket tcp_socket;
	typedef asio::ssl::stream<tcp_socket> ssl_socket;

    enum class ResponseCode : uint8_t
    {
        success = 0,
        disconnect = 1,
        noResponse = 2,
        denied = 3,
        serverError = 4,
    };

    struct Response
    {
        ResponseCode code;
        SerializedData rawData;
        inline InputSerializer data() const
        {
            return {rawData};
        }
    };

	class Connection
	{
	protected:
        AsyncQueue<OMessage> _obuffer;
		AsyncQueue<IMessage> _ibuffer;
		std::shared_mutex _streamLock;
		std::unordered_map<uint32_t, std::function<void(InputSerializer s)>> _streamListeners;
		uint32_t _reqIDCounter = 1000;
		std::shared_mutex _responseLock;
		std::unordered_map<uint32_t, std::function<void(ResponseCode code, InputSerializer s)>> _responseListeners;
		IMessage _tempIn;

		std::vector<std::function<void()>> _onDisconnect;

	public:

		Connection();
		virtual ~Connection();
		virtual void disconnect() = 0;
		virtual bool connected() = 0;

		virtual void send(OMessage&& msg) = 0;
		void sendStreamData(uint32_t id, SerializedData&& sData);
		void endStream(uint32_t id);
		void addStreamListener(uint32_t id, std::function<void(InputSerializer s)> callback);
		void eraseStreamListener(uint32_t id);
		void sendRequest(const std::string& name, SerializedData&& data, const std::function<void(ResponseCode code, InputSerializer s)>& callback);
		void onDisconnect(std::function<void()> f);
		bool messageAvailable();
		IMessage popMessage();

	};

	template<class socket_t>
	class ConnectionBase : public Connection
	{

	protected:
		socket_t _socket;
		std::string _address;
		void async_readHeader()
		{
			_tempIn.body.clear();
			asio::async_read(_socket, asio::buffer(&_tempIn.header, sizeof(MessageHeader)) , [this](std::error_code ec, std::size_t length) {
				if (ec)
                {
                    Runtime::error("[" + _address + "] Header Parse Fail: " + ec.message());
                    disconnect();
                    return;
                }

                if(_tempIn.header.size > 0)
                    async_readBody();
                else
                {
                    _ibuffer.push_back(std::move(_tempIn));
                    async_readHeader();
                }

			});
		}
		void async_readBody()
		{
			_tempIn.body.resize(_tempIn.header.size);
			asio::async_read(_socket, asio::buffer(_tempIn.body.data(), _tempIn.body.size()),  [this](std::error_code ec, std::size_t length) {
				if (ec)
                {
                    Runtime::error("[" + _address + "] Read Message Body Fail: " + ec.message());
                    disconnect();
                    return;
                }
                switch(_tempIn.header.type)
                {
                    case MessageType::response:
                    {
                        InputSerializer s(_tempIn.body);
                        uint32_t id;
                        ResponseCode code;
                        s >> id >> code;

                        std::function<void(ResponseCode code, InputSerializer s)> listener;
                        {
                            std::scoped_lock l(_responseLock);
                            if(!_responseListeners.count(id)){
                                Runtime::error("Unknown response received: " + std::to_string(id));
                                break;
                            }
                            listener = std::move(_responseListeners[id]);
                            _responseListeners.erase(id);
                        }
                        listener(code, s);
                        break;
                    }
                    case MessageType::streamData:
                    {
                        InputSerializer s(_tempIn.body);
                        uint32_t id;
                        s >> id;

                        std::function<void(InputSerializer s)> listener;
                        {
                            std::scoped_lock l(_streamLock);
                            if(!_streamListeners.count(id)){
                                Runtime::error("Unknown stream data received: " + std::to_string(id));
                                break;
                            }
                            listener = std::move(_streamListeners[id]);
                            _streamListeners.erase(id);
                        }
                        listener(s);
                        break;
                    }
                    case MessageType::endStream:
                    {
                        InputSerializer s(_tempIn.body);
                        uint32_t id;
                        s >> id;
                        std::cout << "ending stream: " << id << std::endl;
                        _streamLock.lock();
                        if (_streamListeners.count(id))
                        {
                            _streamListeners.erase(id);
                        } else
                            Runtime::error("Attempted to end nonexistent stream: " + std::to_string(id));
                        _streamLock.unlock();
                        break;
                    }
                    default:
                        _ibuffer.push_back(std::move(_tempIn));
                        break;
                }
                async_readHeader();

			});
		}
		void async_sendMessage()
		{
			asio::async_write(_socket, asio::buffer(&_obuffer.front().header, sizeof(MessageHeader)), [this](std::error_code ec, std::size_t length) {
				if (ec)
				{
                    Runtime::error("[" + _address + "] Write header fail: " + ec.message());
                    disconnect();
                    return;
				}
                async_writeBody();
			});
		}
		void async_writeBody(size_t chunkIndex = 0)
		{
            if(chunkIndex == _obuffer.front().chunks.size())
            {
                _obuffer.pop_front();
                if(!_obuffer.empty())
                    async_sendMessage();
                return;
            }
			asio::async_write(_socket, asio::buffer(_obuffer.front().chunks[chunkIndex].data(), _obuffer.front().chunks[chunkIndex].size()), [this, chunkIndex](std::error_code ec, std::size_t length) {
				if (ec)
                {
                    Runtime::error("[" + _address + "] Write body fail: " + ec.message());
                    disconnect();
                }
                async_writeBody(chunkIndex + 1);
			});
		}
	public:
		ConnectionBase(socket_t&& socket) : _socket(std::move(socket))
		{
		}
		~ConnectionBase()
		{
		}
		void send(OMessage&& msg) override
		{
            msg.header.size = 0;
            for(auto& c : msg.chunks)
                msg.header.size += c.size();

			assert(msg.header.size <= 4294967295); //unsigned int 32 max value
            //mutable so that we can move the messge
			asio::post(_socket.get_executor(), [this, msg = std::move(msg)]() mutable
			{
				bool sending = !_obuffer.empty();
				_obuffer.push_back(std::move(msg));

				if (!sending)
					async_sendMessage();
			});
		}
		void disconnect() override
		{
			asio::post(_socket.get_executor(), [this]{
                _socket.lowest_layer().close();
                for(auto& f: _onDisconnect)
                    f();
                _responseLock.lock();
                SerializedData emptyData;
                InputSerializer s(emptyData);
                for(auto& r: _responseListeners)
                {
                    r.second(ResponseCode::disconnect, s);
                }
                _responseLock.unlock();
			});
		}
		bool connected() override
		{
			return _socket.lowest_layer().is_open();
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
}