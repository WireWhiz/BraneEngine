#pragma once
#include <asio/asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ssl.hpp>

#include <cstdint>
#include <memory>
#include "networkError.h"
#include "config/config.h"
#include "networkQueue.h"
#include "message.h"
#include "networkingComponents.h"
#include <ecs/ecs.h>

namespace net
{

	enum class ConnectionType
	{
		reliable = 0,
		secure = 1,
		fast = 2
	};

	typedef uint32_t ConnectionID;

	class Connection
	{
	public:
		enum class Owner
		{ 
			client,
			server
		};

	protected:
		asio::io_context& _ctx;
		EntityManager* _em;
		NetQueue<OMessage> _obuffer;
		ConnectionID _id;
		Owner _owner;

		IMessage _tempIn;

		virtual void async_readHeader() = 0;
		virtual void async_readBody() = 0;
		virtual void async_writeHeader() = 0;
		virtual void async_writeBody() = 0;

		void addToIMessageQueue();

	public:
		Connection(asio::io_context& ctx, EntityManager* em);

		//Each Connection derived class has it's own unique connect call due to the need to pass a socket
		virtual bool connectToServer(const asio::ip::tcp::resolver::results_type& endpoints) = 0;
		virtual void connectToClient(ConnectionID id) = 0;
		virtual void disconnect() = 0;
		virtual bool isConnected() = 0;
		ConnectionID id();

		virtual void send(const OMessage& msg) = 0;
		virtual ConnectionType type() = 0;

	};

	typedef asio::ip::tcp::socket tcp_socket;
	typedef asio::ssl::stream<tcp_socket> ssl_socket;
	class TCPConnection : public Connection
	{
		tcp_socket* _tcpSocket;
		ssl_socket* _sslSocket;
		bool _secure;
		bool _handshakeDone;
		
		void async_handshake();

	protected:
		void async_readHeader() override;
		void async_readBody() override;
		void async_writeHeader() override;
		void async_writeBody() override;

	public:
		TCPConnection(Owner owner, asio::io_context& ctx, tcp_socket socket, EntityManager* em);
		TCPConnection(Owner owner, asio::io_context& ctx, ssl_socket socket, EntityManager* em);
		~TCPConnection();
		virtual bool connectToServer(const asio::ip::tcp::resolver::results_type& endpoints) override;
		virtual void connectToClient(ConnectionID id) override;
		void disconnect() override;
		bool isConnected() override;

		void send(const OMessage& msg) override;
		ConnectionType type() override;

		
	};
	
	typedef asio::ip::udp::socket udp_socket;
	class FastConnection : public Connection
	{
		udp_socket _socket;
	public:
		void disconnect();
		bool isConnected();

		void send(const OMessage& msg);
		ConnectionType type() override;
	};

	struct NewConnectionComponent : public NativeComponent <NewConnectionComponent>
	{
		REGESTER_MEMBERS_0();
	};

	struct ConnectionComponent : public NativeComponent<ConnectionComponent>
	{
		REGESTER_MEMBERS_2(id, connection);
		ConnectionID id;
		std::shared_ptr<Connection> connection;
		
	};
}