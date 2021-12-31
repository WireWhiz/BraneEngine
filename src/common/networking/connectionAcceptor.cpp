#include "connectionAcceptor.h"


namespace net
{
	ConnectionAcceptor::ConnectionAcceptor(uint16_t port, EntityManager* em) : _acceptor(_ctx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
	{
		_em = em;
		_port = port;
		asyc_waitForConnection();
		ThreadPool::enqueue([&]() {
			_ctx.run();
		});
	}

	ConnectionAcceptor::~ConnectionAcceptor()
	{
		_ctx.stop();
	}

	void ConnectionAcceptor::asyc_waitForConnection()
	{
		_acceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
			if (!ec)
			{
				std::cout << "New Connection: " << socket.remote_endpoint() << std::endl;


				ConnectionComponent cc;
				cc.connection = std::make_shared<ClientConnection<tcp_socket>>(std::move(socket));
				std::shared_ptr<Connection> handle = cc.connection;
				ComponentSet components;
				components.add(ConnectionComponent::def());
				components.add(NewConnectionComponent::def());
				_em->run( [this, cc, components]() mutable {
					EntityID entity = _em->createEntity(components);
					_em->setEntityComponent(entity, cc.toVirtual());
				});
			}
			else
			{
				std::cout << "Connection Error: " << ec.message() << std::endl;
			}

			asyc_waitForConnection();
		});

	}
}