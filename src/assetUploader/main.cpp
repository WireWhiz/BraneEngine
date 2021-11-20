#include <iostream>
#include <string>
#include <networking/connection.h>
#include <ecs/ecs.h>

int main()
{
	std::cout << "Asset Uploader Started" << std::endl;
	EntityManager em;
	// Log in to server

	asio::io_context netCtx;
	net::TCPConnection connection(net::Connection::Owner::client, netCtx, net::tcp_socket(netCtx), &em);

	bool connected = false;
	while (!connected)
	{

		std::string serverAddress;
		std::string port;
		std::cout << "Server Address: ";
		std::cin >> serverAddress;
		std::cout << "port: ";
		std::cin >> port;

		asio::ip::tcp::resolver tcpresolver(netCtx);
		auto tcpEndpoints = tcpresolver.resolve(serverAddress, port);
	}

	//Loop asking for asset to upload
	return 0;
}