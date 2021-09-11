#pragma once
#include "connection.h"
#include <thread>
#include <cstdint>

namespace net
{
	class ClientConnection
	{
		asio::io_context _ctx;
		std::thread _thread;
		std::unique_ptr<Connection> _connection;
		NetQueue<Connection::OwnedIMessage> _messages;
		ConnectionType _type;

	public:
		ClientConnection();
		~ClientConnection();

		bool Connect(const std::string& host, const uint16_t port, const ConnectionType type);
		void Disconnect();

		bool IsConnected();

		NetQueue<Connection::OwnedIMessage>& Incoming();
	};
}