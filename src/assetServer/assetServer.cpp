//
// Created by eli on 3/3/2022.
//

#include "assetServer.h"
#include "common/utility/threadPool.h"

AssetServer::AssetServer() :
_nm(*Runtime::getModule<NetworkManager>()),
_am(*Runtime::getModule<AssetManager>()),
_fm(*Runtime::getModule<FileManager>()),
_db(*Runtime::getModule<Database>())
{
	_nm.start();
	_nm.configureServer();

	_am.setFetchCallback([this](auto id, auto incremental){return fetchAssetCallback(id, incremental);});

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

	_nm.addRequestListener("login", [this](net::RequestResponse& res){
		auto& ctx = getContext(res.sender());
		if(ctx.authenticated)
		{
			res.res() << true;
			res.send();
			return;
		}
		ThreadPool::enqueue([this, res, &ctx]() mutable{
			std::string username, password;
			res.body() >> username >> password;
			if(_db.authenticate(username, password))
			{
				res.res() << true;
				ctx.authenticated = true;
				ctx.username = username;
				ctx.userID = _db.getUserID(username);
				ctx.permissions = _db.userPermissions(ctx.userID);
			}
			else
				res.res() << false;
			res.send();
		});

	});

	_nm.addRequestListener("asset", [this](net::RequestResponse& res){
		auto ctx = getContext(res.sender());
		if(!ctx.authenticated)
			return;
		AssetID id;
		res.body() >> id;
		std::cout << "request for: " << id.string() << std::endl;

		Asset* asset = _am.getAsset<Asset>(id);
		auto f = [this, res](Asset* asset) mutable
		{
			asset->serialize(res.res());
			res.send();
		};

		if(asset)
			f(asset);
		else
			_am.fetchAsset<Asset>(id).then(f);

	});

	_nm.addRequestListener("incrementalAsset", [this](net::RequestResponse& res){
		auto ctx = getContext(res.sender());
		if(!ctx.authenticated)
			return;
		AssetID id;
		uint32_t streamID;
		res.body() >> id >> streamID;
		std::cout << "request for: " << id.string() << std::endl;

		Asset* asset = _am.getAsset<Asset>(id);

		auto f = [this, res, streamID](Asset* asset) mutable
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
		};



		if(asset)
			f(asset);
		else
			_am.fetchAsset<Asset>(id).then(f);

	});

	_nm.addRequestListener("directoryContents", [this](net::RequestResponse& res){
		auto ctx = getContext(res.sender());
		if(!validatePermissions(ctx, {"edit assets"}))
		    return;
		std::string dirPath;
		res.body() >> dirPath;
		if(dirPath.find('.') != std::string::npos)
		{
			res.res() << false;
			res.send();
			return;
		}
		auto contents = _fm.getDirectoryContents(Config::json()["data"]["asset_path"].asString() + "/" + dirPath);
		res.res() << true << contents.directories << contents.files;
		res.send();
	});

	Runtime::timeline().addTask("send asset data", [this]{processMessages();}, "networking");
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

AsyncData<Asset*> AssetServer::fetchAssetCallback(const AssetID& id, bool incremental)
{
	AsyncData<Asset*> asset;
	auto info = _db.getAssetInfo(id.id);
	if(info.filename.empty())
		asset.setError("Asset not found");
	else
	{
		_fm.async_readUnknownAsset(info.filename).then([this, asset, id](Asset* data){
			if(data)
				asset.setData(data);
			else
				asset.setError("Could not read asset: " + std::to_string(id.id));
		});
	}

	return asset;
}

AssetServer::ConnectionContext& AssetServer::getContext(net::Connection* connection)
{
	if(!_connectionCtx.count(connection))
	{
		_connectionCtx.insert({connection, ConnectionContext{}});
		connection->onDisconnect([this, connection]{
			_connectionCtx.erase(connection);
		});
	}

	return _connectionCtx[connection];
}

bool AssetServer::validatePermissions(AssetServer::ConnectionContext& ctx, const std::vector<std::string>& permissions)
{
	for(auto p : permissions)
	{
		if(!ctx.permissions.count(p))
			return false;
	}
	return true;
}


