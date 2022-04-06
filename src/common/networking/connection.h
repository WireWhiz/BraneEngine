#pragma once
#include <asio/asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ssl.hpp>


#include "message.h"
#include <cstdint>
#include <memory>
#include "networkError.h"
#include "config/config.h"
#include "utility/asyncQueue.h"
#include <utility/serializedData.h>
#include <ecs/ecs.h>

namespace net
{

	typedef asio::ip::tcp::socket tcp_socket;
	typedef asio::ssl::stream<tcp_socket> ssl_socket;


	class Connection
	{
	public:

	protected:
		AsyncQueue<std::shared_ptr<OMessage>> _obuffer;
		AsyncQueue<std::shared_ptr<IMessage>> _ibuffer;
		std::shared_ptr<IMessage> _tempIn;
		std::shared_ptr<bool> _exists;

	public:
		Connection();
		~Connection();
		virtual void disconnect() = 0;
		virtual bool connected() = 0;

		virtual void send(std::shared_ptr<OMessage> msg) = 0;
		bool popIMessage(std::shared_ptr<IMessage>& iMessage);

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
						std::cerr << "[" << _address << "] Header Parse Fail: " << ec.message() << std::endl;
						disconnect();
					}
					else
						std::cout << "Socket destroyed" << std::endl;
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
					_ibuffer.push_back(_tempIn);
					async_readHeader();
				}
				else
				{
					if(*exists)
					{
						std::cerr << "[" << _address << "] Read Message Body Fail: " << ec.message() << std::endl;
						disconnect();
					}
					else
						std::cout << "Socket destroyed" << std::endl;
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
					std::cerr << "[" << _address << "] Write header fail: " << ec.message() << std::endl;
					disconnect();
				}
			});
		}
		void async_writeBody()
		{
			std::shared_ptr<bool> exists = _exists;
			asio::async_write(_socket, asio::buffer(_obuffer.front()->body.data.data(), _obuffer.front()->body.data.size()), [this, exists](std::error_code ec, std::size_t length) {
				if(!*exists)
					return;
				if (!ec)
				{
					_obuffer.pop_front();
					if(!_obuffer.empty())
						async_writeHeader();
				}
				else
				{
					std::cerr << "[" << _address << "] Write body fail: " << ec.message() << std::endl;
					disconnect();
				}
			});
		}
	public:
		ConnectionBase(socket_t&& socket) : _socket(std::move(socket))
		{

		}
		~ConnectionBase(){
		}
		void send(std::shared_ptr<OMessage> msg) override
		{
			assert(msg->body.size() <= 65535); //unsigned int 16 max value
			msg->header.size = msg->body.size();
			std::shared_ptr<bool> exists = _exists;
			asio::post(_socket.get_executor(), [this, msg, exists]() {
				if(!*exists)
					return;
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
					_socket.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both);
					_socket.lowest_layer().close();

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
		void connectToServer(const asio::ip::tcp::resolver::results_type& endpoints, std::function<void()> onConnect);
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