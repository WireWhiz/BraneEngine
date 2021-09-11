#include "server.h"


namespace net
{
	ServerInterface::ServerInterface(uint16_t port) 
		: _tcpAcceptor(_ctx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
	{
		_port = port;
	}
	ServerInterface::~ServerInterface()
	{
		stop();
	}
	bool ServerInterface::start()
	{
		try
		{
			asyc_waitForConnection();

			_thread = std::thread([this]() {
				_ctx.run();
			});
		}
		catch (const std::exception& e)
		{
			std::cerr << "Server Error: " << e.what() << std::endl;
			return false;
		}

		return true;
	}
	void ServerInterface::stop()
	{
		_ctx.stop();
		if (_thread.joinable())
			_thread.join();
	}
	void ServerInterface::asyc_waitForConnection()
	{
		_tcpAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
			if (!ec)
			{
				std::cout << "New Connection: " << socket.remote_endpoint() << std::endl;

				//std::shared_ptr<Connection> newConnection = std::make_shared<ReliableConnection>(Connection::Owner::server, _ctx, std::move(socket), _imessages);
				

			}
			else
			{
				std::cout << "Connection Error: " << ec.message() << std::endl;
			}

			asyc_waitForConnection();
		});
	}
}