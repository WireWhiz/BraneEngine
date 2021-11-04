#pragma once
#include <cstdint>
#include <ecs/ecs.h>
#include <asio/asio.hpp>
#include "connection.h"
#include <utility/threadPool.h>


namespace net
{

	class ConnectionAcceptor
	{
		EntityManager* _em;

		asio::io_context _ctx;
		uint16_t _port;
		asio::ip::tcp::acceptor _acceptor;


		void asyc_waitForConnection();
	public:
		ConnectionAcceptor(uint16_t port, EntityManager* em);
		~ConnectionAcceptor();
	};
}