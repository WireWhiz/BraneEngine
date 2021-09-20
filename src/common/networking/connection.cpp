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
	void ReliableConnection::async_readHeader()
	{
		asio::async_read(_socket, asio::buffer(&_tempIn.header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length) 
			{
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
					_socket.close();
				}
				
			});
	}
	void ReliableConnection::async_readBody()
	{
		asio::async_read(_socket, asio::buffer(_tempIn.data.data(), _tempIn.data.size()),
			[this](std::error_code ec, std::size_t length) {
				if (!ec)
				{
					addToIMessageQueue();
					async_readHeader();
				}
				else
				{
					std::cout << "[" << _id << "] Read message body fail: " << ec.message() << std::endl;
					_socket.close();
				}
			});
	}
	void ReliableConnection::async_writeHeader()
	{
		asio::async_write(_socket, asio::buffer(&_obuffer.front().header, sizeof(MessageHeader)),
			[this](std::error_code ec, std::size_t length) 
			{
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
					_socket.close();
				}
			});
	}
	void ReliableConnection::async_writeBody()
	{
		asio::async_write(_socket, asio::buffer(_obuffer.front().data.data(), _obuffer.front().data.size()),
						  [this](std::error_code ec, std::size_t length) {
			if (!ec)
			{
				_obuffer.pop_front();
				if (!_obuffer.empty())
					async_writeHeader();
			}
			else
			{
				std::cout << "[" << _id << "] Write body fail: " << ec.message() << std::endl;
				_socket.close();
			}
		});
	}
	ReliableConnection::ReliableConnection(Owner owner, asio::io_context& ctx, tcp_socket& socket, NetQueue<OwnedIMessage>& ibuffer) : Connection(ctx, ibuffer), _socket(std::move(socket))
	{
		_owner = owner;
	}
	bool ReliableConnection::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		assert(_owner == Owner::client);
		asio::async_connect(_socket, endpoints,
			[this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
				if (!ec)
				{
					async_readHeader();
				}
				else
				{
					std::cerr << "Failed to connect to server.";
					return false;
				}
			});
		
		
		return true;
	}
	void ReliableConnection::connectToClient(ConnectionID id)
	{
		assert(_owner == Owner::server);
		if (_socket.is_open())
		{
			_id = id;
			async_readHeader();
		}
		
	}
	void ReliableConnection::dissconnect()
	{
		_socket.close();
	}
	bool ReliableConnection::isConnected()
	{
		return _socket.is_open();
	}
	void ReliableConnection::send(const OMessage& msg)
	{

		asio::post(_ctx, [this, msg]() {
			bool sending = !_obuffer.empty();
			_obuffer.push_back(msg);
			if (!sending)
				async_writeHeader();
		});
	}
	ConnectionType ReliableConnection::type()
	{
		return ConnectionType::reliable;
	}
	void SecureConnection::async_handshake()
	{
		asio::ssl::stream_base::handshake_type base;
		if (_owner == Owner::server)
			base = asio::ssl::stream_base::server;
		else
			base = asio::ssl::stream_base::client;

		
		_socket.async_handshake(base, [this](std::error_code ec) {
			if (!ec || ec == asio::error::eof)
			{
				async_readHeader();
			}
			else
			{
				std::cout << "SSL handshake failed: " << ec.message() << std::endl;
			}
		});
	}
	void SecureConnection::async_readHeader()
	{
		asio::async_read(_socket, asio::buffer(&_tempIn.header, sizeof(MessageHeader)),
						 [this](std::error_code ec, std::size_t length) {
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
				_socket.next_layer().close();
			}

		});
	}
	void SecureConnection::async_readBody()
	{
		asio::async_read(_socket, asio::buffer(_tempIn.data.data(), _tempIn.data.size()),
						 [this](std::error_code ec, std::size_t length) {
			if (!ec)
			{
				addToIMessageQueue();
				async_readHeader();
			}
			else
			{
				std::cout << "[" << _id << "] Read message body fail: " << ec.message() << std::endl;
				_socket.next_layer().close();
			}
		});
	}
	void SecureConnection::async_writeHeader()
	{
		asio::async_write(_socket, asio::buffer(&_obuffer.front().header, sizeof(MessageHeader)),
						  [this](std::error_code ec, std::size_t length) {
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
				_socket.next_layer().close();
			}
		});
	}
	void SecureConnection::async_writeBody()
	{
		asio::async_write(_socket, asio::buffer(_obuffer.front().data.data(), _obuffer.front().data.size()),
						  [this](std::error_code ec, std::size_t length) {
			if (!ec)
			{
				_obuffer.pop_front();
				if (!_obuffer.empty())
					async_writeHeader();
			}
			else
			{
				std::cout << "[" << _id << "] Write body fail: " << ec.message() << std::endl;
				_socket.next_layer().close();
			}
		});
	}
	SecureConnection::SecureConnection(Owner owner, asio::io_context& ctx, ssl_socket& socket, NetQueue<OwnedIMessage>& ibuffer) : Connection(ctx, ibuffer), _socket(std::move(socket))
	{
		_owner = owner;
	}
	bool SecureConnection::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
	{
		assert(_owner == Owner::client);
		asio::async_connect(_socket.next_layer(), endpoints,
							[this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
			if (!ec)
			{
				async_handshake();
			}
			else
			{
				std::cout << "Failed to connect to server: " << ec.message() << std::endl;
				return false;
			}
		});


		return true;
	}
	void SecureConnection::connectToClient(ConnectionID id)
	{
		assert(_owner == Owner::server);
		if (_socket.next_layer().is_open())
		{
			_id = id;
			async_handshake();
		}
	}
	void SecureConnection::dissconnect()
	{
		_socket.next_layer().close();
	}
	bool SecureConnection::isConnected()
	{
		return _socket.next_layer().is_open();
	}
	void SecureConnection::send(const OMessage& msg)
	{

		asio::post(_ctx, [this, msg]() {
			bool sending = !_obuffer.empty();
			_obuffer.push_back(msg);
			if (!sending)
				async_writeHeader();
		});
	}
	ConnectionType SecureConnection::type()
	{
		return ConnectionType::secure;
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