#include <iostream>
#include <string>
#include <networking/connection.h>
#include <ecs/ecs.h>
#include <chrono>

void async_acceptConnections(asio::ip::tcp::acceptor& acceptor, std::shared_ptr<net::ServerConnection<net::tcp_socket>>& server)
{
	acceptor.async_accept([&](std::error_code ec, asio::ip::tcp::socket socket)
	 {
	     if (!ec)
	     {
	         std::cout << "New Connection: " << socket.remote_endpoint() << std::endl;
	         server = std::make_shared<net::ServerConnection<net::tcp_socket>>(std::move(socket));
	         server->connectToClient();
	     }
	     else
	     {
	         std::cout << "Connection Error: " << ec.message() << std::endl;
	     }
	 });
}

int main()
{
	std::cout << "Asset Uploader Started" << std::endl;
	EntityManager em;
	// Log in to server
	asio::io_context ctx;
	asio::ssl::context ssl_context(asio::ssl::context::tls);

	net::tcp_socket receiving_socket(ctx);

	asio::ip::tcp::resolver resolver(ctx);
	auto endpoints = resolver.resolve("localhost", "2022");

	net::ClientConnection<net::tcp_socket> client = net::ClientConnection<net::tcp_socket>(net::tcp_socket(ctx));
	asio::ip::tcp::acceptor acceptor(ctx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 2022));

	std::shared_ptr<net::ServerConnection<net::tcp_socket>> server;
	async_acceptConnections(acceptor, server);
	std::thread t([&](){ctx.run();});

	client.connectToServer(endpoints);

	while(server == nullptr)
	{
		std::cout << "Waiting for connection" << std::endl;
	}
	std::cout << "Connection accepted, sending message" << std::endl;

	std::shared_ptr<net::OSerializedData> testOMessage = std::make_shared<net::OSerializedData>();
	*testOMessage << std::string("This is working");
	server->send(testOMessage);

	std::shared_ptr<net::ISerializedData> testMessage;
	while(!client.popIMessage(testMessage))
	{
		std::cout << "Waiting for message" << std::endl;
	}
	std::string testMessageData;
	*testMessage >> testMessageData;
	std::cout << "Message received: " << testMessageData << std::endl;

	//net::TCPConnection connection(net::Connection::Owner::client, netCtx, net::tcp_socket(netCtx), &em);

	//bool connected = false;
	/*while (!connected)
	{

		std::string serverAddress;
		std::string port;
		std::cout << "Server Address: ";
		std::cin >> serverAddress;
		std::cout << "port: ";
		std::cin >> port;

		asio::ip::tcp::resolver tcpresolver(netCtx);
		auto tcpEndpoints = tcpresolver.resolve(serverAddress, port);
	}*/

	//Loop asking for asset to upload

	ctx.stop();
	t.join();
	return 0;
}