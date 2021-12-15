#include "connection.h"

namespace net
{
	/* //For reference when adding messages to ecs
	void Connection::addToIMessageQueue()
	{
		ComponentSet components;
		components.add(IMessageComponent::def());

		IMessageComponent im;
		im.message = std::move(_tempIn);
		im.owner = _id;

		_em->run([this, components, im]() mutable {
			EntityID entity = _em->createEntity(components);
			_em->setEntityComponent(entity, im.toVirtual());
		});
	}
	 */



	bool Connection::popIMessage(std::shared_ptr<IMessage>& iMessage)
	{
		if(!_ibuffer.empty())
		{
			iMessage = _ibuffer.pop_front();
			return true;
		}
		return false;
	}

	template<>
	void ServerConnection<tcp_socket>::connectToClient()
	{
		if(connected())
			async_readHeader();
	}

	template<>
	void ServerConnection<ssl_socket>::connectToClient()
	{
		if(!connected())
			return;
		_socket.async_handshake(asio::ssl::stream_base::server, [this](std::error_code ec) {
			if (!ec)
			{
				async_readHeader();
			}
			else
			{
				std::cout << "SSL handshake failed: " << ec.message() << std::endl;
			}
		});

	}

	template<>
	void ClientConnection<tcp_socket>::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		asio::async_connect(_socket.lowest_layer(), endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
			if (!ec)
			{

				async_readHeader();
			}
			else
			{
				std::cerr << "Failed to connect to server.";
			}
		});
	}

	template<>
	void ClientConnection<ssl_socket>::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		asio::async_connect(_socket.lowest_layer(), endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
			if (!ec)
			{
				_socket.async_handshake(asio::ssl::stream_base::client, [this](std::error_code ec) {
					if (!ec)
					{
						async_readHeader();
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
	/*void TCPConnection::async_handshake()
	{
		assert(_secure);
		assert(_sslSocket);
		asio::ssl::stream_base::handshake_type base;
		if (_owner == Owner::server)
			base = asio::ssl::stream_base::server;
		else
			base = asio::ssl::stream_base::client;

		_sslSocket->async_handshake(base, [this](std::error_code ec) {
			if (!ec)
			{
				_handshakeDone = true;
				async_readHeader();
			}
			else
			{
				std::cout << "SSL handshake failed: " << ec.message() << std::endl;
			}
		});
	}
	TCPConnection::TCPConnection(Owner owner, asio::io_context& ctx, tcp_socket socket, EntityManager* em) : Connection(ctx, em)
	{
		_tcpSocket = new tcp_socket(std::move(socket));
		_sslSocket = nullptr;
		_owner = owner;
		_secure = false;
		_handshakeDone = true;
	}

	TCPConnection::~TCPConnection()
	{
		if (_tcpSocket)
			delete _tcpSocket;
		if (_sslSocket)
			delete _sslSocket;
	}
	bool TCPConnection::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		assert(_owner == Owner::client);
		auto callback = [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
			if (!ec)
			{
				if (_secure)
					async_handshake();
				else
					async_readHeader();
			}
			else
			{
				std::cerr << "Failed to connect to server.";
			}
		};
		if(_secure)
			asio::async_connect(_sslSocket->lowest_layer(), endpoints, callback);
		else 
			asio::async_connect(*_tcpSocket, endpoints, callback);
		
		return true;
	}
	void TCPConnection::connectToClient(ConnectionID id)
	{
		assert(_owner == Owner::server);
		if (isConnected())
		{
			_id = id;
			if (_secure)
				async_handshake();
			else
				async_readHeader();
		}
		
	}
	void TCPConnection::disconnect()
	{
		if (_secure)
			return _sslSocket->lowest_layer().close();
		else
			return _tcpSocket->close();
	}
	bool TCPConnection::isConnected()
	{
		if (_secure)
			return _sslSocket->lowest_layer().is_open();
		else
			return _tcpSocket->is_open();
	}
	void TCPConnection::send(const OMessage&& msg)
	{
		//We can't send messages until handshake is complete, so just requeue this function until it is.
		if (!_handshakeDone)
		{
			if(isConnected()) // If the handshake failed we will disconnect, therefore we shouldn't requeue;
				asio::post(_ctx, [this, msg]() {
					send(msg);
				});
			return;
		};
		asio::post(_ctx, [this, msg]() {
			bool sending = !_obuffer.empty();
			_obuffer.push_back(msg);
			if (!sending)
				async_writeHeader();
		});
	}*/


}