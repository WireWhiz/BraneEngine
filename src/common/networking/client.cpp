#include "client.h"

namespace net
{
	ClientConnection::ClientConnection()
	{

	}
	ClientConnection::~ClientConnection()
	{
		Disconnect();
	}

	bool ClientConnection::Connect(const std::string& host, const uint16_t port, const ConnectionType type)
	{
		_type = type;
		try
		{
			asio::ip::tcp::resolver tcpresolver(_ctx);
			asio::ip::basic_resolver_results<asio::ip::tcp> tcpEndpoints; 

			asio::ip::udp::resolver udpresolver(_ctx);
			asio::ip::basic_resolver_results<asio::ip::udp> udpEndpoints;

			switch (type)
			{
				case ConnectionType::reliable:
					tcpEndpoints = tcpresolver.resolve(host, std::to_string(port));

					

					_connection = std::make_unique<ReliableConnection>(Connection::Owner::client, std::make_unique<tcp_socket>(_ctx));
					_connection->connect(host, port);
					break;
				default:
					throw std::runtime_error("Unimplemented connection type " + std::to_string((int)type) + "was used");
			}

			_thread = std::thread([this]() {_ctx.run(); });
		}
		catch (const std::exception& e)
		{
			std::cerr << "Client connection failed: " << e.what() << std::endl;
			return false;
		}
		return true;
	}

	void ClientConnection::Disconnect()
	{
		if (_connection)
			if (_connection->isConnected())
				_connection->dissconnect();

		_ctx.stop();

		if (_thread.joinable())
			_thread.join();

		_connection.release();
	}

	bool ClientConnection::IsConnected()
	{
		if (!_connection)
			return false;
		return _connection->isConnected();
	}

	NetQueue<Connection::OwnedIMessage>& ClientConnection::Incoming()
	{
		return _messages;
	}
}

