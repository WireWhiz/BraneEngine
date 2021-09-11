#include "connection.h"

namespace net
{
	ReliableConnection::ReliableConnection(std::unique_ptr<tcp_socket>& socket)
	{
		_socket = std::move(socket);
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