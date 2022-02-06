#pragma once
#include <asio/asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ssl.hpp>

#include <iostream>
#include <thread>
#include <vector>
#include <memory>

#include "serializedData.h"
#include "connection.h"
#include "config/config.h"
#include "networkError.h"


namespace net
{

	class NetworkConnection
	{
	public:
		NetworkConnection()
		{
			asio::error_code ec;
			asio::io_context context;

			asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);

			asio::ip::tcp::socket socket(context);

			socket.connect(endpoint, ec);
			

			if (!ec)
				std::cout << "Connected!" << std::endl;
			else
				std::cout << "Failed to connect: " << ec.message() << std::endl;

			if (socket.is_open())
			{
				std::string sRequest = "GET /INDEX.HTML HTTP/1.1\r\n"
					"Host: example.com\r\n"
					"Connection: close\r\n\r\n";

				socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);
			}

			socket.wait(socket.wait_read);
			size_t bytes = socket.available();
			std::cout << "Receved response: " << bytes << std::endl;


			if (bytes > 0)
			{
				std::vector<char> vBuffer(bytes);
				socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
				for (char c : vBuffer)
					std::cout << c;
			}
			
		}
	};

	class NetworkManager
	{
		asio::io_context _context;
		std::vector<std::unique_ptr<Connection>> _connections;
	public:
		NetworkManager();
		~NetworkManager();
		void openListener(const uint32_t port);
		void openConnection();

	};
}