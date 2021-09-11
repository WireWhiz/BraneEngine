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

namespace net
{

	enum class ConnectionType
	{
		reliable = 0,
		secure = 1,
		fast = 2
	};

	

	class Connection
	{
	public:
		struct OwnedIMessage
		{
			IMessage message;
			std::shared_ptr<Connection> owner;
		};

		struct OwnedOMessage
		{
			OMessage message;
			std::shared_ptr<Connection> owner;
		};

	protected:
		NetQueue<OwnedIMessage> _ibuffer;
		NetQueue<OwnedOMessage> _obuffer;

	public:

		//Each Connection derived class has it's own unique connect call due to the need to pass a socket
		virtual bool dissconnect() = 0;
		virtual bool isConnected() = 0;

		virtual bool send(const OMessage& msg) = 0;
		virtual ConnectionType type() = 0;

	};

	typedef asio::ip::tcp::socket tcp_socket;
	class ReliableConnection : public Connection
	{
		std::unique_ptr<tcp_socket> _socket;
	public:
		ReliableConnection(std::unique_ptr<tcp_socket>& socket);
		virtual bool dissconnect();
		virtual bool isConnected();

		virtual bool send(const OMessage& msg);
		ConnectionType type() override;
	};

	typedef asio::ssl::stream<tcp_socket> ssl_socket;
	class SecureConnection : public Connection
	{
		
		ssl_socket _socket;
	public:
		SecureConnection(ssl_socket socket);
		virtual bool dissconnect();
		virtual bool isConnected();

		virtual bool send(const OMessage& msg);
		ConnectionType type() override;

	};
	
	typedef asio::ip::udp::socket udp_socket;
	class FastConnection : public Connection
	{
		udp_socket _socket;
	public:
		virtual bool dissconnect();
		virtual bool isConnected();

		virtual bool send(const OMessage& msg) = 0;
		ConnectionType type() override;
	};
	
}