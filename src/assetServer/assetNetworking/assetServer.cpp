//
// Created by eli on 3/3/2022.
//

#include "assetServer.h"

AssetServer::AssetServer(Runtime& runtime) : Module(runtime), _nm(*(NetworkManager*)runtime.getModule("networkManager")), _am(*(AssetManager*)runtime.getModule("assetManager"))
{
	_nm.start();
	_am.isServer = true;
	_nm.configureServer();

	if(!Config::json()["network"]["use_ssl"].asBool())
	{
		std::cout << "Started listening for asset requests on port: " << Config::json()["network"]["tcp_port"].asUInt() << std::endl;
		_nm.openClientAcceptor<net::tcp_socket>(Config::json()["network"]["tcp_port"].asUInt(), [this](const std::unique_ptr<net::Connection>& connection){
			std::cout << "User connected to tcp" << std::endl;
		});
	}
	else
	{
		std::cout << "Started listening for asset requests on port: " << Config::json()["network"]["ssl_port"].asUInt() << std::endl;
		_nm.openClientAcceptor<net::ssl_socket>(Config::json()["network"]["ssl_port"].asUInt(), [this](const std::unique_ptr<net::Connection>& connection){
			std::cout << "User connected to ssl" << std::endl;
		});
	}

	_nm.addRequestListener("asset", [this](net::RequestResponse& res){
		AssetID id;
		res.body() >> id;
		std::cout << "request for: " << id.string() << std::endl;

		Asset* asset = nullptr;
		try{
			asset = _am.getAsset<Asset>(id);

		}
		catch(const std::exception& e)
		{
			std::cerr << "Failed to read asset: " << e.what();
		}

		if(asset)
		{
			asset->serialize(res.res());
			res.send();
		}

	});

	_nm.addRequestListener("incrementalAsset", [this](net::RequestResponse& res){
		AssetID id;
		uint32_t streamID;
		res.body() >> id >> streamID;
		std::cout << "request for: " << id.string() << std::endl;

		Asset* asset = nullptr;
		try{
			asset = _am.getAsset<Asset>(id);

		}
		catch(const std::exception& e)
		{
			std::cerr << "Failed to read asset: " << e.what();
		}

		if(asset)
		{
			auto* ia = dynamic_cast<IncrementalAsset*>(asset);
			if(ia)
			{
				std::cout<< "Sending header for: " << ia->id << std::endl;
				ia->serializeHeader(res.res());


				res.send();

				IncrementalAssetSender assetSender{};
				assetSender.iteratorData = ia->createContext();
				assetSender.asset = ia;
				assetSender.streamID = streamID;
				assetSender.connection = res.sender();
				_senders.push_back(std::move(assetSender));
			}
			else
				std::cerr << "Tried to request non-incremental asset as incremental" << std::endl;
		}

	});
	runtime.timeline().addTask("send asset data", [this]{processMessages();}, "networking");
}

void AssetServer::processMessages()
{
	//Send one increment from every incremental asset that we are sending, to create the illusion of them loading in parallel
	_senders.remove_if([&](IncrementalAssetSender& sender)
	{
		try
		{
			OSerializedData data;
			bool moreData = sender.asset->serializeIncrement(data, sender.iteratorData.get());
			sender.connection->sendStreamData(sender.streamID, std::move(data));
			if(!moreData)
				sender.connection->sendStreamEnd(sender.streamID);
			return !moreData;
		}
		catch(const std::exception& e) {
			std::cerr << "Asset sender error: " << e.what() << std::endl;
			return true;
		}
	});
}

AssetServer::~AssetServer()
{

}

const char* AssetServer::name()
{
	return "assetServer";
}
