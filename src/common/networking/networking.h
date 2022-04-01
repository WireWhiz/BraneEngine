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

class NetworkManager
{
	asio::io_context _context;

	std::shared_ptr<JobHandle> _threadHandle;
	asio::ssl::context _ssl_context;
	asio::ip::tcp::resolver _tcpResolver;

	std::shared_mutex _assetServerLock;
	std::unordered_map<std::string, std::shared_ptr<net::Connection>> _assetServers;

	std::shared_mutex _runtimeServerLock;
	std::unordered_map<std::string, std::shared_ptr<net::Connection>> _runtimeServers;

	std::vector<asio::ip::tcp::acceptor> _acceptors;

	std::atomic_bool _running;

	std::mutex _listenersLock;
	std::unordered_map<AssetID, std::function<void(Asset* asset)>> _assetLoadListeners;
	std::unordered_map<AssetID, std::function<void(IncrementalAsset* asset)>> _assetHeaderListeners;
	std::unordered_map<AssetID, std::function<void(ISerializedData& sData)>> _assetIncrementListeners;

	void connectToAssetServer(std::string ip, uint16_t port);
	void async_connectToAssetServer(std::string ip, uint16_t port, std::function<void()> callback);
	template<typename socket_t>
	void async_acceptConnections(size_t acceptor, std::function<void (std::unique_ptr<net::Connection>&& connection)> callback)
	{
		socket_t* socket;
		if constexpr(std::is_same<socket_t, net::tcp_socket>().value)
			socket = new net::tcp_socket(_context);
		if constexpr(std::is_same<socket_t, net::ssl_socket>().value)
			socket = new net::ssl_socket(_context, _ssl_context);
		_acceptors[acceptor].async_accept(socket->lowest_layer(), [this, acceptor, socket, callback](std::error_code ec) {
			if (!ec)
			{
				callback(std::make_unique<net::ServerConnection<socket_t>>(std::move(*socket)));

			}
			else
			{
				std::cout << "Connection Error: " << ec.message() << std::endl;
			}
			delete socket;
			async_acceptConnections<socket_t>(acceptor, callback);
		});
	}
public:
	NetworkManager();
	~NetworkManager();
	void start();
	void stop();
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