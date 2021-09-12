#include "connection.h"

namespace net
{
	ReliableConnection::ReliableConnection(Owner owner, std::unique_ptr<tcp_socket>& socket)
	{
		_owner = owner;
		_socket = std::move(socket);
	}
	bool ReliableConnection::connect(const std::string& ip, uint16_t port)
	{
		try
		{
			asio::ip::tcp::endpoint ep(asio::ip::make_address_v4(ip), port);
			_socket->connect(ep);
		}
		catch (const std::exception& e)
		{
			std::cout << "Could not connect to " << ip << " on port " << port << std::endl << e.what() << std::endl;
			return false;
		}
		
		return true;
	}
	void ReliableConnection::dissconnect()
	{
		_socket->close();
	}
	bool ReliableConnection::isConnected()
	{
		return _socket->is_open();
	}
	bool ReliableConnection::send(const OMessage& msg)
	{
		return false;
	}
	ConnectionType ReliableConnection::type()
	{
		return ConnectionType::reliable;
	}
	ConnectionType SecureConnection::type()
	{
		return ConnectionType::secure;
	}
	ConnectionType FastConnection::type()
	{
		return ConnectionType::fast;
	}
}