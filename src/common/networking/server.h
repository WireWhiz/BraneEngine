#pragma once
#include "connection.h"
#include <vector>
#include <thread>

namespace net
{
	class ServerInterface
	{
	protected:
		asio::io_context _ctx;
		asio::ssl::context _ssl_ctx;
		std::thread _thread;
		NetQueue<std::shared_ptr<Connection>> _connections;
		//NetQueue<Connection::OwnedIMessage> _imessages;
		ConnectionType _type;
		uint16_t _port;

		asio::ip::tcp::acceptor _acceptor;
		asio::ip::tcp::acceptor _sslAcceptor;

		uint32_t _idCounter = 10000;
	public:
		ServerInterface(uint16_t port, uint16_t ssl_port);

		~ServerInterface();

		bool start();
		void stop();
		void update(size_t maxMessages = -1);


		void asyc_waitForConnection();
		void asyc_waitForSSLConnection();
		void messageClient(std::shared_ptr<Connection> client, const OMessage& msg);
		void messageAll(const OMessage& msg, std::shared_ptr<Connection> ignore = nullptr);

	protected:
		virtual bool onClientConnect(std::shared_ptr<Connection> client) = 0;
		virtual void onClientDissconnect(std::shared_ptr<Connection> client) = 0;
		virtual void onMessage(std::shared_ptr<Connection> client, IMessage& msg) = 0;
	};
}
