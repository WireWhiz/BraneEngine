#include "networking.h"
#include "assets/assetManager.h"
#include "request.h"
#include <atomic>

NetworkManager::NetworkManager(Runtime& runtime) : _tcpResolver(_context), _ssl_context(asio::ssl::context::tls), Module(runtime)
{
	_running = false;
	startSystems(runtime);
}

NetworkManager::~NetworkManager()
{

}

void NetworkManager::connectToAssetServer(std::string ip, uint16_t port)
{
	std::cout << "Connecting to asset server: " << ip << ":" << port << std::endl;
	auto* connection = new net::ClientConnection<net::tcp_socket>(net::tcp_socket(_context));

	auto tcpEndpoints = _tcpResolver.resolve(ip, std::to_string(port));
	connection->connectToServer(tcpEndpoints, [this, ip, connection]() mutable{
		_assetServerLock.lock();
		_assetServers.insert({ip, std::unique_ptr<net::ClientConnection<net::tcp_socket>>(connection)});
		_assetServerLock.unlock();
	} );


}



void NetworkManager::async_connectToAssetServer(const std::string& address, uint16_t port, const std::function<void(bool)>& callback)
{
	_assetServerLock.lock_shared();
	if(_assetServers.count(address))
	{
		callback(true);
		_assetServerLock.unlock_shared();
		return;
	}
	_assetServerLock.unlock_shared();

    std::cout << "connecting to: " << address << ":" << port << std::endl;
	_tcpResolver.async_resolve(asio::ip::tcp::resolver::query(address, std::to_string(port), asio::ip::tcp::resolver::query::canonical_name), [this, address, callback](const asio::error_code ec, auto endpoints){
		if(!ec)
		{
            std::cout << "Connected to asset server: " << address << std::endl;
			auto* connection = new net::ClientConnection<net::tcp_socket>(net::tcp_socket(_context));
			connection->connectToServer(endpoints, [this, address, callback, connection](){
				_assetServerLock.lock();
				_assetServers.insert({address, std::unique_ptr<net::Connection>(connection)});
				_assetServerLock.unlock();
				callback(true);
				if(!_running)
					start();
			});
		}
		else
			callback(false);

	});
	if(!_running)
		start();

}

void NetworkManager::start()
{
	_running = true;
	_threadHandle = ThreadPool::addStaticThread([this](){
		while(_running)
            _context.run();
		std::cout << "exiting networking thread\n";
	});
}

void NetworkManager::stop()
{
	std::cout << "Shutting down networking... ";
	for(auto& connection : _assetServers)
		connection.second->disconnect();
	_running = false;
    _context.stop();
    if(_threadHandle)
	    _threadHandle->finish();

	_assetServers.clear();

	std::cout << "done" << std::endl;
}

void NetworkManager::configureServer()
{
	if(Config::json()["network"]["use_ssl"].asBool())
	{
		try{
			_ssl_context.set_options(
					asio::ssl::context::default_workarounds
					| asio::ssl::context::no_sslv2
					| asio::ssl::context::single_dh_use
			);
			_ssl_context.use_certificate_chain_file(Config::json()["network"]["ssl_cert"].asString());
			_ssl_context.use_private_key_file(Config::json()["network"]["private_key"].asString(), asio::ssl::context::pem);
			_ssl_context.use_tmp_dh_file(Config::json()["network"]["tmp_dh"].asString());
		}
		catch (const std::exception& e)
		{
			std::cerr << "Couldn't read file: " << e.what() << std::endl;
		}
	}
}

AsyncData<Asset*> NetworkManager::async_requestAsset(const AssetID& id, AssetManager& am)
{
	AsyncData<Asset*> asset;

	_assetServerLock.lock_shared();
	if(!_assetServers.count(id.serverAddress))
		throw std::runtime_error("No connection with " + id.serverAddress);
	net::Connection* server = _assetServers[id.serverAddress].get();
	_assetServerLock.unlock_shared();

	std::cout << "async requesting: " << id.string() << std::endl;

	// Set up a listener for the asset response
	OSerializedData o;
	o << id;
	net::Request req("asset", std::move(o));

	server->sendRequest(req).then([asset, &am](ISerializedData&& sData){
		asset.setData(Asset::deserializeUnknown(sData, am));
	});
	return asset;
}


AsyncData<IncrementalAsset*> NetworkManager::async_requestAssetIncremental(const AssetID& id, AssetManager& am)
{
	AsyncData<IncrementalAsset*> asset;

	_assetServerLock.lock_shared();
	net::Connection* server = _assetServers[id.serverAddress].get();
	_assetServerLock.unlock_shared();

	std::cout << "async requesting incremental: " << id.string() << std::endl;
	uint32_t streamID = _streamIDCounter++;
	OSerializedData o;
	o << id << streamID;
	net::Request req("incrementalAsset", std::move(o));

	// Set up a listener for the asset response
	server->sendRequest(req).then([this, asset, server, streamID, &am](ISerializedData sData){
		IncrementalAsset* assetPtr = IncrementalAsset::deserializeUnknownHeader(sData, am);
		asset.setData(assetPtr);
		server->addStreamListener(streamID, [assetPtr](ISerializedData& sData){
			assetPtr->deserializeIncrement(sData);
		});
	});
	return asset;

}

void NetworkManager::startSystems(Runtime& rt)
{
	rt.timeline().addTask("ingest data", [this]{
		_assetServerLock.lock_shared();
		for(auto& s : _assetServers){
			ingestData(s.second.get());
		}
		_assetServerLock.unlock_shared();
		_runtimeServerLock.lock_shared();
		for(auto& s : _runtimeServers){
			ingestData(s.second.get());
		}
		_runtimeServerLock.unlock_shared();
		_clientLock.lock_shared();
		for(auto& s : _clients){
			if(s)
				ingestData(s.get());
		}
		_clientLock.unlock_shared();
	}, "networking");

}

const char* NetworkManager::name()
{
	return "networkManager";
}

void NetworkManager::addRequestListener(const std::string& name, std::function<void(net::RequestResponse&)> callback)
{
	std::scoped_lock l(_requestLock);
	assert(!_requestListeners.count(name));
	_requestListeners.insert({name, callback});
}

void NetworkManager::ingestData(net::Connection* connection)
{
	while(connection->messageAvailable())
	{
		std::shared_ptr<net::IMessage> message = connection->popMessage();
		switch(message->header.type)
		{
			case net::MessageType::request:
			{
				std::string name;
				net::RequestResponse response(connection, message);
				{
					std::scoped_lock l(_requestLock);
					if(!_requestListeners.count(response.name())){
						std::cout << "Unknown request received: " << response.name() << std::endl;
						break;
					}
					_requestListeners[response.name()](response);
				}
				break;
			}
			default:
				std::cerr << "Received message of unknown type: " << (int)message->header.type << std::endl;
				break;
		}
	}
}
