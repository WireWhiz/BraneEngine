#include "networking.h"
#include "assets/assetManager.h"
#include <atomic>

NetworkManager::NetworkManager() : _tcpResolver(_context), _ssl_context(asio::ssl::context::tls)
{
	_running = false;
}

NetworkManager::~NetworkManager()
{

	std::cout << "Shutting down networking... ";
	stop();
	std::cout << "done" << std::endl;

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



void NetworkManager::async_connectToAssetServer(std::string ip, uint16_t port, std::function<void()> callback)
{


 std::cout << "connecting to: " << ip << ":" << port << std::endl;
	_tcpResolver.async_resolve(asio::ip::tcp::resolver::query(ip, std::to_string(port), asio::ip::tcp::resolver::query::canonical_name),[this, ip, callback](const asio::error_code ec, auto endpoints){
		if(!ec)
		{
            std::cout << "Connected to asset server: " << ip << std::endl;
			auto connection = std::make_shared<net::ClientConnection<net::tcp_socket>>(net::tcp_socket(_context));
			connection->connectToServer(endpoints, [this, ip, callback, connection](){
				_assetServerLock.lock();
				_assetServers.insert({ip,connection});
				_assetServerLock.unlock();
				callback();
			});
		}
		else
			throw std::runtime_error("Could not resolve asset server address: " + ec.message());

	});

    if(!_running)
        start();



}

void NetworkManager::start()
{
	_threadHandle = ThreadPool::addStaticThread([this](){
        _context.run();
		std::cout << "exiting networking thread\n";
        _running = false;
	});
}

void NetworkManager::stop()
{
	for(auto& connection : _assetServers)
		connection.second->disconnect();
    _context.stop();
    if(_threadHandle)
	    _threadHandle->finish();

	_assetServers.clear();
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

void NetworkManager::async_requestAsset(const AssetID& id, AssetManager& am, AsyncData<Asset*> asset)
{
	assert(asset.callbackSet());
	auto f = [this, id, asset]{
		_assetServerLock.lock_shared();
		net::Connection* server = _assetServers[id.serverAddress].get();
		_assetServerLock.unlock_shared();

		std::cout << "async requesting: " << id.string() << std::endl;

		// Set up a listener for the asset response
		_listenersLock.lock();
		_assetLoadListeners.insert({id, [asset](Asset* data){
			asset.setData(data);
		}});
		_listenersLock.unlock();

		AssetRequest ar{};
		ar.id = id;
		ar.incremental = false;
		server->send(ar.toMessage());
	};

	if(!_assetServers.count(id.serverAddress))
		async_connectToAssetServer(id.serverAddress, Config::json()["network"]["tcp_port"].asUInt(), f);
	else
		f();
}


void NetworkManager::async_requestAssetIncremental(const AssetID& id, AssetManager& am, AsyncData<IncrementalAsset*> asset)
{
	assert(asset.callbackSet());
	auto f = [this, id, asset]{
		_assetServerLock.lock_shared();
		net::Connection* server = _assetServers[id.serverAddress].get();
		_assetServerLock.unlock_shared();

		std::cout << "async requesting incremental: " << id.string() << std::endl;

		// Set up a listener for the asset response
		_listenersLock.lock();
		_assetHeaderListeners.insert({id, [asset](IncrementalAsset* data){
			asset.setData(data);
		}});
		_listenersLock.unlock();

		AssetRequest ar{};
		ar.id = id;
		ar.incremental = true;
		server->send(ar.toMessage());
	};

	if(!_assetServers.count(id.serverAddress))
		async_connectToAssetServer(id.serverAddress, Config::json()["network"]["tcp_port"].asUInt(), f);
	else
		f();

}

void NetworkManager::startAssetAcceptorSystem(EntityManager& em, AssetManager& am)
{
	std::unique_ptr<VirtualSystem> vs = std::make_unique<VirtualSystem>(AssetID("nativeSystem/2"), [this, &am](EntityManager& em){
		for(auto& s : _assetServers){
			std::shared_ptr<net::IMessage> message;
			while(s.second->popIMessage(message))
			{
				switch(message->header.type)
				{
					case net::MessageType::assetData:
					{
						ThreadPool::enqueue([this, message, &am](){
							Asset* asset;
							try{
								asset = Asset::deserializeUnknown(message->body, am);
							}
							catch(const std::exception& e)
							{
								std::cerr << "Problem deserializing asset: " << e.what() << std::endl;
								return;
							}

							_listenersLock.lock();
							if(!_assetLoadListeners.count(asset->id)){
								std::cout << "Unknown asset received: " << asset->name << std::endl;
								return;
							}
							auto f = std::move(_assetLoadListeners[asset->id]); //We can't call this from inside the lock, since callback can also request assets, and that also requires a lock
							_assetLoadListeners.erase(asset->id);
							_listenersLock.unlock();

							f(asset);
						});
					}
						break;
					case net::MessageType::assetIncrementalHeader:
					{
						IncrementalAsset* asset = nullptr;
						try{
							asset = IncrementalAsset::deserializeUnknownHeader(message->body, am);
						}
						catch(const std::exception& e)
						{
							std::cerr << "Problem deserializing asset header: " << e.what() << std::endl;
							return;
						}

						_listenersLock.lock();
						auto f = _assetHeaderListeners[asset->id]; //We can't call this from inside the lock, since callback can also request assets, and that also requires a lock
						_assetHeaderListeners.erase(asset->id);


						_assetIncrementListeners.insert({
							asset->id,
							[asset](ISerializedData& sData){
								asset->deserializeIncrement(sData);
							}
						});
						_listenersLock.unlock();

						f(asset);
					}
						break;
					case net::MessageType::assetIncrementalData:
					{
						AssetID id;
						message->body >> id;
						ThreadPool::enqueue([this, id, message](){
							_listenersLock.lock();
							auto f = _assetIncrementListeners[id]; //We can't call this from inside the lock, since callback can also request assets, and that also requires a lock
							//TODO: increment listeners need to erase themselves after asset is fully loaded
							_listenersLock.unlock();

							f(message->body);
							//TODO: increment listeners need to erase themselves after asset is fully loaded
						});
					}
						break;
					default:
						std::cerr << "Received message of unknown type: " << (int)message->header.type << std::endl;
						break;
				}
			}
		}
	});
	assert(em.addSystem(std::move(vs)));
}




std::shared_ptr<net::OMessage> AssetRequest::toMessage()
{
	std::shared_ptr<net::OMessage> message = std::make_shared<net::OMessage>();
	message->header.type = net::MessageType::assetRequest;
	message->body << id << incremental;
	return message;
}

void AssetRequest::fromMessage(std::shared_ptr<net::IMessage> message)
{
	assert(message->header.type == net::MessageType::assetRequest);
	message->body >> id >> incremental;
}
