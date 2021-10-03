#include "connection.h"

namespace net
{
	void Connection::addToIMessageQueue()
	{
		if (_owner == Owner::server)
			_ibuffer.push_back({ sharedThis, _tempIn });
		else
			_ibuffer.push_back({ nullptr, _tempIn });

	}
	Connection::Connection(asio::io_context& ctx, NetQueue<OwnedIMessage>& ibuffer) : _ctx(ctx), _ibuffer(ibuffer)
	{
		sharedThis = std::shared_ptr<Connection>(this);
	}
	void TCPConnection::async_handshake()
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
	void TCPConnection::async_readHeader()
	{
		auto callback = [this](std::error_code ec, std::size_t length) {
			if (!ec)
			{
				if (_tempIn.header.size > 0)
				{
					_tempIn.data.resize(_tempIn.header.size);
					async_readBody();
				}
				else
				{
					addToIMessageQueue();
					async_readHeader();
				}

			}
			else
			{
				std::cout << "[" << _id << "] Header Parse Fail: " << ec.message() << std::endl;
				disconnect();
			}

		};

		if(_secure)
			asio::async_read(*_sslSocket, asio::buffer(&_tempIn.header, sizeof(MessageHeader)), callback);
		else
			asio::async_read(*_tcpSocket, asio::buffer(&_tempIn.header, sizeof(MessageHeader)), callback);

	}
	void TCPConnection::async_readBody()
	{
		auto callback = [this](std::error_code ec, std::size_t length) {
			if (!ec)
			{
				addToIMessageQueue();
				async_readHeader();
			}
			else
			{
				std::cout << "[" << _id << "] Read message body fail: " << ec.message() << std::endl;
				disconnect();
			}
		};
		if(_secure)
			asio::async_read(*_sslSocket, asio::buffer(_tempIn.data.data(), _tempIn.data.size()), callback);
		else
			asio::async_read(*_tcpSocket, asio::buffer(_tempIn.data.data(), _tempIn.data.size()), callback);

	}
	void TCPConnection::async_writeHeader()
	{
		auto callback = [this](std::error_code ec, std::size_t length) {
			if (!ec)
			{
				if (_obuffer.front().data.size() > 0)
					async_writeBody();
				else
				{
					_obuffer.pop_front();

					if (!_obuffer.empty())
						async_writeHeader();
				}

			}
			else
			{
				std::cout << "[" << _id << "] Write header fail: " << ec.message() << std::endl;
				disconnect();
			}
		};
		if (_secure)
			asio::async_write(*_sslSocket, asio::buffer(&_obuffer.front().header, sizeof(MessageHeader)), callback);
		else
			asio::async_write(*_tcpSocket, asio::buffer(&_obuffer.front().header, sizeof(MessageHeader)), callback);

	}
	void TCPConnection::async_writeBody()
	{
		auto callback = [this](std::error_code ec, std::size_t length) {
			if (!ec)
			{
				_obuffer.pop_front();
				if (!_obuffer.empty())
					async_writeHeader();
			}
			else
			{
				std::cout << "[" << _id << "] Write body fail: " << ec.message() << std::endl;
				disconnect();
			}
		};
		if (_secure)
			asio::async_write(*_sslSocket, asio::buffer(_obuffer.front().data.data(), _obuffer.front().data.size()), callback);
		else
			asio::async_write(*_tcpSocket, asio::buffer(_obuffer.front().data.data(), _obuffer.front().data.size()), callback);

	}
	TCPConnection::TCPConnection(Owner owner, asio::io_context& ctx, tcp_socket& socket, NetQueue<OwnedIMessage>& ibuffer) : Connection(ctx, ibuffer)
	{
		_tcpSocket = new tcp_socket(std::move(socket));
		_sslSocket = nullptr;
		_owner = owner;
		_secure = false;
		_handshakeDone = true;
	}
	TCPConnection::TCPConnection(Owner owner, asio::io_context& ctx, ssl_socket& socket, NetQueue<OwnedIMessage>& ibuffer) : Connection(ctx, ibuffer)
	{
		_sslSocket = new ssl_socket(std::move(socket));
		_tcpSocket = nullptr;
		_owner = owner;
		_secure = true;
		_handshakeDone = false;
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
				return false;
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
	void TCPConnection::send(const OMessage& msg)
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
	}
	ConnectionType TCPConnection::type()
	{
		if(_secure)
			return ConnectionType::secure;
		else
			return ConnectionType::reliable;
	}
	ConnectionType FastConnection::type()
	{
		return ConnectionType::fast;
	}
	
	ConnectionID Connection::id()
	{
		return _id;
	}

}