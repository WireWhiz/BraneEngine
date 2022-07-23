//
// Created by eli on 3/3/2022.
//

#include "assetServer.h"

AssetServer::AssetServer() :
_nm(*Runtime::getModule<NetworkManager>()),
_am(*Runtime::getModule<AssetManager>()),
_fm(*Runtime::getModule<FileManager>()),
_db(*Runtime::getModule<Database>())
{
	_nm.start();
	_nm.configureServer();

	_am.setFetchCallback([this](auto id, auto incremental){return fetchAssetCallback(id, incremental);});
	std::filesystem::create_directory(Config::json()["data"]["asset_path"].asString());

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

	_nm.addRequestListener("login", [this](auto* connection, InputSerializer req, OutputSerializer res){
		auto& ctx = getContext(connection);
		if(ctx.authenticated)
		{
			res << true;
			return;
		}
        std::string username, password;
        req >> username >> password;
        if(_db.authenticate(username, password))
        {
            res << true;
            ctx.authenticated = true;
            ctx.username = username;
            ctx.userID = _db.getUserID(username);
            ctx.permissions = _db.userPermissions(ctx.userID);
        }
        else
            res << false;

	});

	_nm.addRequestListener("asset", [this](auto* connection, InputSerializer req, OutputSerializer res){
		auto ctx = getContext(connection);
		if(!ctx.authenticated)
			return;
		AssetID id;
		req >> id;
		std::cout << "request for: " << id.string() << std::endl;

		Asset* asset = _am.getAsset<Asset>(id);
		auto f = [this, res](Asset* asset) mutable
		{
			asset->serialize(res);
		};

		if(asset)
			f(asset);
		else
			_am.fetchAsset<Asset>(id).then(f);

	});

	_nm.addRequestListener("incrementalAsset", [this](auto* connection, InputSerializer req, OutputSerializer res){
		auto ctx = getContext(connection);
		if(!ctx.authenticated)
			return;
		AssetID id;
		uint32_t streamID;
		req >> id >> streamID;
		std::cout << "request for: " << id.string() << std::endl;

		Asset* asset = _am.getAsset<Asset>(id);

		auto f = [this, res, connection, streamID](Asset* asset) mutable
		{
			auto* ia = dynamic_cast<IncrementalAsset*>(asset);
			if(ia)
			{
				std::cout<< "Sending header for: " << ia->id << std::endl;
				ia->serializeHeader(res);

				IncrementalAssetSender assetSender{};
				assetSender.iteratorData = ia->createContext();
				assetSender.asset = ia;
				assetSender.streamID = streamID;
				assetSender.connection = connection;
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

	_nm.addRequestListener("processAsset",[this](auto* connection, InputSerializer req, OutputSerializer res){
		auto ctx = getContext(connection);
		if(!validatePermissions(ctx, {"edit assets"}))
			return;
		std::string assetFilename;   //Name of raw file
		std::string assetName;       //desired asset name
		std::string assetPath;       //desired asset path
		std::string assetData; //file data
		req >> assetFilename >> assetName >> assetPath >> assetData;

		if(!checkAssetPath(assetPath))
		{
			res << false;
			return;
		}
		std::string assetDirectory = Config::json()["data"]["asset_path"].asString();


		res << true;
	});

	addDirectoryRequestListeners();
	Runtime::timeline().addTask("send asset data", [this]{processMessages();}, "networking");
}


void AssetServer::addDirectoryRequestListeners()
{
	_nm.addRequestListener("directoryContents", [this](auto* connection, InputSerializer req, OutputSerializer res){
		auto ctx = getContext(connection);
		if(!validatePermissions(ctx, {"edit assets"}))
			return;
		std::string dirPath;
		req >> dirPath;
		if(!checkAssetPath(dirPath))
		{
			res << false;
			return;
		}
		auto contents = _fm.getDirectoryContents(Config::json()["data"]["asset_path"].asString() + "/" + dirPath);
		res << true << contents.directories << static_cast<uint16_t>(contents.files.size());
		for(auto& f : contents.files)
		{
			res << f;
			AssetID id;
			bool isAsset = _db.fileToAssetID(dirPath + f, id);
			res << isAsset;
			if(isAsset)
			{
				id.serverAddress = Config::json()["network"]["domain"].asString();
				res<< id;
			}
		}
	});

	_nm.addRequestListener("createDirectory", [this](auto* connection, InputSerializer req, OutputSerializer res){
		auto ctx = getContext(connection);
		if(!validatePermissions(ctx, {"edit assets"}))
			return;
		std::string dirPath;
		req >> dirPath;
		if(!checkAssetPath(dirPath))
		{
			res << false;
			return;
		}
		_fm.createDirectory(Config::json()["data"]["asset_path"].asString() + "/" + dirPath);
		res<< true;
	});

	_nm.addRequestListener("moveDirectory", [this](auto* connection, InputSerializer req, OutputSerializer res){
		auto ctx = getContext(connection);
		if(!validatePermissions(ctx, {"edit assets"}))
			return;
		std::string dirSrc, dirDest;
		req >> dirSrc >> dirDest;
		if(!checkAssetPath(dirSrc) || !checkAssetPath(dirDest))
		{
			res << false;
			return;
		}
		_fm.moveFile(Config::json()["data"]["asset_path"].asString() + "/" + dirSrc, Config::json()["data"]["asset_path"].asString() + "/" + dirDest);
		res << true;
	});

	_nm.addRequestListener("deleteFile", [this](auto* connection, InputSerializer req, OutputSerializer res){
		auto ctx = getContext(connection);
		if(!validatePermissions(ctx, {"edit assets"}))
			return;
		std::string dirPath;
		req >> dirPath;
		if(!checkAssetPath(dirPath))
		{
			res << false;
			return;
		}
		bool notRoot = false;
		for (char c : dirPath)
		{
			if(std::isalpha(c) || std::isdigit(c))
			{
				notRoot = true;
				break;
			}
		}
		if(notRoot)
			_fm.deleteFile(Config::json()["data"]["asset_path"].asString() + "/" + dirPath);
		res << notRoot;
	});
}

void AssetServer::processMessages()
{
	//Send one increment from every incremental asset that we are sending, to create the illusion of them loading in parallel
	_senders.remove_if([&](IncrementalAssetSender& sender)
	{
		try
		{
			SerializedData data;
            OutputSerializer s(data);
			bool moreData = sender.asset->serializeIncrement(s, sender.iteratorData.get());
			sender.connection->sendStreamData(sender.streamID, std::move(data));
			if(!moreData)
                sender.connection->endStream(sender.streamID);
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
		_fm.async_readUnknownAsset(Config::json()["data"]["asset_path"].asString() + "/" + info.filename).then([this, asset, id](Asset* data){
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
	if(ctx.userID == 1)
		return true; //Admin has all permissions;
	for(auto p : permissions)
	{
		if(!ctx.permissions.count(p))
			return false;
	}
	return true;
}

bool AssetServer::checkAssetPath(const std::string& path)
{
	return path.find('.') == std::string::npos;
}



