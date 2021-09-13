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
	void ServerInterface::update(size_t maxMessages)
	{
		size_t messageCount = 0;
		while (messageCount < maxMessages && !_imessages.empty())
		{
			Connection::OwnedIMessage msg = _imessages.pop_front();
			onMessage(msg.owner, msg.message); //TODO make it so that on message is called by a thread pool
			messageCount++;
		}
	}
	void ServerInterface::asyc_waitForConnection()
	{
		_tcpAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
			if (!ec)
			{
				std::cout << "New Connection: " << socket.remote_endpoint() << std::endl;

				std::shared_ptr<Connection> newConnection = std::make_shared<ReliableConnection>(Connection::Owner::server, _ctx, std::move(socket), _imessages);
				
				if (onClientConnect(newConnection))
				{
					_connections.push_back(std::move(newConnection));
					_connections.back()->connectToClient(_idCounter++);
					std::cout << "[" << _connections.back()->id() << "] Connection Accepted" << std::endl;
				}
				else
					std::cout << "Connection denied" << std::endl;
			}
			else
			{
				std::cout << "Connection Error: " << ec.message() << std::endl;
			}

			asyc_waitForConnection();
		});
	}
	void ServerInterface::messageClient(std::shared_ptr<Connection> client, const OMessage& msg)
	{
		if (client && client->isConnected())
		{
			client->send(msg);
		}
		else
		{
			onClientDissconnect(client);
			client.reset();
			_connections.erase(client);



		}
	}
	void ServerInterface::messageAll(const OMessage& msg, std::shared_ptr<Connection> ignore)
	{
		bool invalidClientExists = false;
		for (auto& client : _connections)
		{
			if (client && client->isConnected())
			{
				client->send(msg);
			}
			else
			{
				onClientDissconnect(client);
				client.reset();
				invalidClientExists = true;
				
			}

			if(invalidClientExists)
				_connections.clean();
		}

	}
}