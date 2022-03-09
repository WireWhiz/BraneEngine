#include "networking.h"


NetworkManager::NetworkManager() : _ssl_context(asio::ssl::context::tls)
{

}

NetworkManager::~NetworkManager()
{
	if(!_context.stopped())
		_context.stop();
}

void NetworkManager::connectToAssetServer(std::string ip, uint16_t port)
{
	std::cout << "Connecting to asset server: " << ip << ":" << port << std::endl;
	std::unique_ptr<net::ClientConnection<net::tcp_socket>> connection = std::make_unique<net::ClientConnection<net::tcp_socket>>(net::tcp_socket(_context));

	asio::ip::tcp::resolver tcpresolver(_context);
	auto tcpEndpoints = tcpresolver.resolve(ip, std::to_string(port)); //TODO make this async
	connection->connectToServer(tcpEndpoints);

	_assetServers.insert({ip, std::move(connection)});
}

Asset* NetworkManager::requestAsset(AssetID& id, AssetManager& am)
{
	assert(!_context.stopped());
	if(!_assetServers.count(id.serverAddress))
		connectToAssetServer(id.serverAddress, Config::json()["network"]["tcp_port"].asUInt());

	net::Connection* server = _assetServers[id.serverAddress].get();
	std::shared_ptr<net::OMessage> o = std::make_shared<net::OMessage>();
	o->header.type = net::MessageType::assetRequest;
	o->body << id.string();
	while(!server->connected())
	{
		std::cout << "waiting to connect to server\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::cout << "requesting: " << id.string() << std::endl;
	server->send(o);

	std::shared_ptr<net::IMessage> res;
	while(!server->popIMessage(res))
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	assert(res->header.type == net::MessageType::assetData);
	Asset* asset = Asset::deserializeUnknown(res->body, am);
	return asset;
}

Asset* NetworkManager::requestAssetIncremental(AssetID& id, AssetManager& am)
{
	assert(!_context.stopped());
	if(!_assetServers.count(id.serverAddress))
		connectToAssetServer(id.serverAddress, Config::json()["networking"]["ssl_port"].asUInt());



	return nullptr;
}

void NetworkManager::start()
{
	ThreadPool::addStaticThread([this](){
        std::function<void()> keepContextRunning;
		keepContextRunning = [&](){
	        asio::post(_context, [&](){
				keepContextRunning();
	        });
	    };
	    keepContextRunning();
		_context.run();
	});
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


