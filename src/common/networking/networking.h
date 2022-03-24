#pragma once
#include <asio/asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ssl.hpp>

#include <iostream>
#include <thread>
#include <utility>
#include <vector>
#include <memory>

#include "serializedData.h"
#include "connection.h"
#include "config/config.h"
#include "networkError.h"
#include <shared_mutex>



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
	asio::ssl::context _ssl_context;
	std::shared_mutex _assetServerLock;
	std::unordered_map<std::string, std::unique_ptr<net::Connection>> _assetServers;
	std::shared_mutex _runtimeServerLock;
	std::unordered_map<std::string, std::unique_ptr<net::Connection>> _runtimeServers;
	std::vector<asio::ip::tcp::acceptor> _acceptors;

	std::mutex _listenersLock;
	std::unordered_map<AssetID, std::function<void(Asset* asset)>> _assetLoadListeners;
	std::unordered_map<AssetID, std::function<void(IncrementalAsset* asset)>> _assetHeaderListeners;
	std::unordered_map<AssetID, std::function<void(ISerializedData& sData)>> _assetIncrementListeners;

	void connectToAssetServer(std::string ip, uint16_t port);
	void async_connectToAssetServer(std::string ip, uint16_t port, std::function<void()> callback);
	template<typename socket_t>
	void async_acceptConnections(size_t acceptor, std::function<void (std::unique_ptr<net::Connection>&& connection)> callback)
	{
		std::cout << "Listening for another connection" << std::endl;
		socket_t* socket;
		if constexpr(std::is_same<socket_t, net::tcp_socket>().value)
			socket = new net::tcp_socket(_context);
		if constexpr(std::is_same<socket_t, net::ssl_socket>().value)
			socket = new net::ssl_socket(_context, _ssl_context);
		_acceptors[acceptor].async_accept(socket->lowest_layer(), [this, acceptor, socket, callback](std::error_code ec) {
			if (!ec)
			{
				callback(std::make_unique<net::ServerConnection<socket_t>>(std::move(*socket)));
				delete socket;
			}
			else
			{
				std::cout << "Connection Error: " << ec.message() << std::endl;
			}
			async_acceptConnections<socket_t>(acceptor, callback);
		});
	}
public:
	NetworkManager();
	~NetworkManager();
	void start();
	void configureServer();

	template<typename socket_t>
	void openAcceptor(const uint32_t port, std::function<void (std::unique_ptr<net::Connection>&& connection)> callback)
	{
		_acceptors.emplace_back(_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
		auto& acceptor = *(_acceptors.end() - 1);
		async_acceptConnections<socket_t>(_acceptors.size()-1, callback);
	}

	void startAssetAcceptorSystem(EntityManager& em, AssetManager& am);

	void async_requestAsset(const AssetID& id, AssetManager& am, AsyncData<Asset*> asset);
	void async_requestAssetIncremental(const AssetID& id, AssetManager& am, AsyncData<IncrementalAsset*> asset);
};

struct AssetRequest
{
	AssetID id;
	bool incremental = false;
	std::shared_ptr<net::OMessage> toMessage();
	void fromMessage(std::shared_ptr<net::IMessage> message);
};