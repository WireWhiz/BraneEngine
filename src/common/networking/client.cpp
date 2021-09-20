#include "client.h"

namespace net
{
	ClientConnection::ClientConnection() : _ssl_ctx(asio::ssl::context::tls)
	{

	}
	ClientConnection::~ClientConnection()
	{
		disconnect();
	}

	bool ClientConnection::connect(const std::string& host, const uint16_t port, const ConnectionType type)
	{
		_type = type;
		try
		{

			switch (type)
			{
				case ConnectionType::reliable:
				{
					asio::ip::tcp::resolver tcpresolver(_ctx);
					auto tcpEndpoints = tcpresolver.resolve(host, std::to_string(port));
					_connection = std::make_unique<ReliableConnection>(Connection::Owner::client, _ctx, tcp_socket(_ctx), _imessages);
					_connection->connectToServer(tcpEndpoints);
				}
					break;
				case ConnectionType::secure:
				{
					asio::ip::tcp::resolver tcpresolver(_ctx);
					auto tcpEndpoints = tcpresolver.resolve(host, std::to_string(port));
					_connection = std::make_unique<SecureConnection>(Connection::Owner::client, _ctx, ssl_socket(_ctx, _ssl_ctx), _imessages);
					_connection->connectToServer(tcpEndpoints);
				}
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

	void ClientConnection::disconnect()
	{
		if (_connection)
			if (_connection->isConnected())
				_connection->dissconnect();

		_ctx.stop();

		if (_thread.joinable())
			_thread.join();

		_connection.release();
	}

	bool ClientConnection::connected()
	{
		if (!_connection)
			return false;
		return _connection->isConnected();
	}

	void ClientConnection::send(const OMessage& msg)
	{
		_connection->send(msg);
	}

	NetQueue<Connection::OwnedIMessage>& ClientConnection::incoming()
	{
		return _imessages;
	}
}

