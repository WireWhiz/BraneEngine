#include "connection.h"

namespace net
{

	bool Connection::popIMessage(std::shared_ptr<IMessage>& iMessage)
	{
		if(!_ibuffer.empty())
		{
			iMessage = _ibuffer.pop_front();
			return true;
		}
		return false;
	}

	Connection::Connection()
	{
		_exists = std::make_shared<bool>(true);
	}

	Connection::~Connection()
	{
		*_exists = false;
	}

	template<>
	void ServerConnection<tcp_socket>::connectToClient()
	{
		_address = _socket.remote_endpoint().address().to_string();
		async_readHeader();
	}

	template<>
	void ServerConnection<ssl_socket>::connectToClient()
	{
		_socket.async_handshake(asio::ssl::stream_base::server, [this](std::error_code ec) {
			if (!ec)
			{
				_address = _socket.lowest_layer().remote_endpoint().address().to_string();
				async_readHeader();
			}
			else
			{
				std::cout << "SSL handshake failed: " << ec.message() << std::endl;
			}
		});

	}

	template<>
	void ClientConnection<tcp_socket>::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints, std::function<void()> onConnect)
	{

		std::shared_ptr<bool> exists = _exists;
		asio::async_connect(_socket.lowest_layer(), endpoints, [this, onConnect, exists](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
			if (!ec && *exists)
			{
				_address = _socket.remote_endpoint().address().to_string();
				async_readHeader();
				onConnect();
			}
			else
			{
				std::cerr << "Failed to connect to server.";
			}
		});
	}

	template<>
	void ClientConnection<ssl_socket>::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints, std::function<void()> onConnect)
	{
		std::shared_ptr<bool> exists = _exists;
		asio::async_connect(_socket.lowest_layer(), endpoints, [this, exists, onConnect = std::move(onConnect)](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
			if (!ec && *exists)
			{
				_address = _socket.lowest_layer().remote_endpoint().address().to_string();
				_socket.async_handshake(asio::ssl::stream_base::client, [this, exists, onConnect = std::move(onConnect)](std::error_code ec) {
					if (!ec && *exists)
					{
						async_readHeader();
						onConnect();
					}
					else
					{
						std::cout << "SSL handshake failed: " << ec.message() << std::endl;
					}
				});
			}
			else
			{
				std::cerr << "Failed to connect to server.";
			}
		});
	}
}