#pragma once
#include "connection.h"
#include <thread>
#include <cstdint>
/*

namespace net
{
	class ClientConnection
	{
		asio::io_context _ctx;
		asio::ssl::context _ssl_ctx;
		std::thread _thread;
		std::unique_ptr<Connection> _connection;
		//NetQueue<OwnedIMessage> _imessages;
		NetQueue<OMessage> _omessages;
		ConnectionType _type;

	public:
		ClientConnection();
		~ClientConnection();

		bool connect(const std::string& host, const uint16_t port, const ConnectionType type);
		void disconnect();

		bool connected();

		void send(const OMessage& msg);

		//NetQueue<OwnedIMessage>& incoming();
	};
}*/
