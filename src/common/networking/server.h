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
		std::thread _thread;
		std::vector<std::shared_ptr<Connection>> _connections;
		NetQueue<Connection::OwnedIMessage> _imessages;
		ConnectionType _type;
		uint16_t _port;

		asio::ip::tcp::acceptor _tcpAcceptor;

		uint32_t _idCounter = 10000;
	public:
		ServerInterface(uint16_t port);

		~ServerInterface();

		bool start();
		void stop();


		void asyc_waitForConnection();
		void messageClient(std::shared_ptr<Connection> client, const OMessage& msg);
		void messageAll(const OMessage& msg, std::shared_ptr<Connection> ignore = nullptr);

	protected:
		virtual bool onClientConnect(std::shared_ptr<Connection> client) = 0;
		virtual void onClientDissconnect(std::shared_ptr<Connection> client) = 0;
		virtual void onMessage(std::shared_ptr<Connection> client, IMessage& msg) = 0;
	};
}
