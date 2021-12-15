#include "client.h"

/*
namespace net
{
	ClientConnection::ClientConnection() : _ssl_ctx(asio::ssl::context::tls)
	{
		_ssl_ctx.set_options(
			asio::ssl::context::default_workarounds
			| asio::ssl::context::no_sslv2
			| asio::ssl::context::single_dh_use
		);
		_ssl_ctx.load_verify_file("keys\\rootca.crt");
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
					//_connection = std::make_unique<TCPConnection>(Connection::Owner::client, _ctx, tcp_socket(_ctx), _imessages);
					_connection->connectToServer(tcpEndpoints);
				}
					break;
				case ConnectionType::secure:
				{
					asio::ip::tcp::resolver tcpresolver(_ctx);
					auto tcpEndpoints = tcpresolver.resolve(host, std::to_string(port));
					//_connection = std::make_unique<TCPConnection>(Connection::Owner::client, _ctx, ssl_socket(_ctx, _ssl_ctx), _imessages);
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
				_connection->disconnect();

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

	*/
/*NetQueue<OwnedIMessage>& ClientConnection::incoming()
	{
		return _imessages;
	}*//*

}

*/
