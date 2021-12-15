#pragma once
#include <asio/asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ssl.hpp>

#include <cstdint>
#include <memory>
#include "networkError.h"
#include "config/config.h"
#include "networkQueue.h"
#include "message.h"
#include "networkingComponents.h"
#include <ecs/ecs.h>

namespace net
{
	typedef uint32_t ConnectionID;

	typedef asio::ip::tcp::socket tcp_socket;
	typedef asio::ssl::stream<tcp_socket> ssl_socket;

	class Connection
	{
	public:

	protected:
		EntityManager* _em;
		NetQueue<std::shared_ptr<OMessage>> _obuffer;
		NetQueue<std::shared_ptr<IMessage>> _ibuffer;
		std::shared_ptr<IMessage> _tempIn;

	public:
		virtual void disconnect() = 0;
		virtual bool connected() = 0;

		virtual void send(std::shared_ptr<OMessage> msg) = 0;
		virtual bool popIMessage(std::shared_ptr<IMessage>& iMessage);

	};

	template<class socket_t>
	class ConnectionBase : public Connection
	{
	protected:
		socket_t _socket;
		void async_readHeader()
		{
			_tempIn = std::make_shared<IMessage>();
			asio::async_read(_socket, asio::buffer(&_tempIn->header, sizeof(MessageHeader)),[this](std::error_code ec, std::size_t length) {
				if (!ec)
				{
					if (_tempIn->header.size > 0)
					{
						_tempIn->data.resize(_tempIn->header.size);
						async_readBody();
					}
					else
					{
						_ibuffer.push_back(_tempIn);
						_tempIn = nullptr;
						async_readHeader();
					}

				}
				else
				{
					std::cout << "[" << _socket.lowest_layer().remote_endpoint().address().to_string() << "] Header Parse Fail: " << ec.message() << std::endl;
					disconnect();
				}

			});
		}
		void async_readBody()
		{
			asio::async_read(_socket, asio::buffer(_tempIn->data.data(), _tempIn->data.size()),  [this](std::error_code ec, std::size_t length) {
				if (!ec)
				{
					_ibuffer.push_back(_tempIn);
					_tempIn = nullptr;
					async_readHeader();
				}
				else
				{
					std::cout << "[" << _socket.lowest_layer().remote_endpoint().address().to_string() << "] Read message body fail: " << ec.message() << std::endl;
					disconnect();
				}
			});
		}
		void async_writeHeader()
		{
			asio::async_write(_socket, asio::buffer(&_obuffer.front()->header, sizeof(MessageHeader)), [this](std::error_code ec, std::size_t length) {
				if (!ec)
				{
					if (_obuffer.front()->data.size() > 0)
						async_writeBody();
					else
					{
						_obuffer.pop_front();

						if (!_obuffer.empty())
							async_writeHeader();
					}

				}
				else
				{
					std::cout << "[" << _socket.lowest_layer().remote_endpoint().address().to_string() << "] Write header fail: " << ec.message() << std::endl;
					disconnect();
				}
			});
		}
		void async_writeBody()
		{
			asio::async_write(_socket, asio::buffer(_obuffer.front()->data.data(), _obuffer.front()->data.size()), [this](std::error_code ec, std::size_t length) {
				if (!ec)
				{
					_obuffer.pop_front();
					if (!_obuffer.empty())
						async_writeHeader();
				}
				else
				{
					std::cout << "[" << _socket.lowest_layer().remote_endpoint().address().to_string() << "] Write body fail: " << ec.message() << std::endl;
					disconnect();
				}
			});
		}
	public:
		ConnectionBase(socket_t&& socket) : _socket(std::move(socket))
		{

		}
		void send(std::shared_ptr<OMessage> msg) override
		{
			asio::post(_socket.get_executor(), [this, msg]() {
				bool sending = !_obuffer.empty();
				_obuffer.push_back(msg);
				if (!sending)
					async_writeHeader();
			});
		}
		void disconnect() override
		{
			_socket.lowest_layer().close();
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
		void connectToServer(const asio::ip::tcp::resolver::results_type& endpoints);
	};



	struct NewConnectionComponent : public NativeComponent <NewConnectionComponent>
	{
		REGISTER_MEMBERS_0();
	};

	struct ConnectionComponent : public NativeComponent<ConnectionComponent>
	{
		REGISTER_MEMBERS_2(id, connection);
		ConnectionID id;
		std::shared_ptr<Connection> connection;
		
	};
}