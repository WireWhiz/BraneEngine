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

	_nm.addRequestListener("login", [this](auto& rc){
		auto& ctx = getContext(rc.sender);
		if(ctx.authenticated)
			return;
        rc.code = net::ResponseCode::denied;
        std::string username, password;
        rc.req >> username >> password;
        if(_db.authenticate(username, password))
        {
            ctx.authenticated = true;
            ctx.username = username;
            ctx.userID = _db.getUserID(username);
            ctx.permissions = _db.userPermissions(ctx.userID);
            rc.code = net::ResponseCode::success;
        }

	});

	_nm.addRequestListener("asset", [this](auto& rc){
		auto ctx = getContext(rc.sender);
		if(!ctx.authenticated)
        {
            rc.code = net::ResponseCode::denied;
            return;
        }
		AssetID id;
		rc.req >> id;
		std::cout << "request for: " << id.string() << std::endl;



		Asset* asset = _am.getAsset<Asset>(id);
		if(asset)
            asset->serialize(rc.res);
		else
        {
            auto ctxPtr = std::make_shared<RequestCTX>(std::move(rc));
            _am.fetchAsset<Asset>(id).then([ctxPtr](Asset* asset) mutable
            {
                asset->serialize(ctxPtr->res);
            });

        }

	});

	_nm.addRequestListener("incrementalAsset", [this](auto& rc){
		auto ctx = getContext(rc.sender);
		if(!ctx.authenticated)
        {
            rc.code = net::ResponseCode::denied;
            return;
        }
		AssetID id;
		uint32_t streamID;
		rc.req >> id >> streamID;
		std::cout << "request for: " << id.string() << std::endl;

		Asset* asset = _am.getAsset<Asset>(id);
        auto ctxPtr = std::make_shared<RequestCTX>(std::move(rc));
		auto f = [this, ctxPtr, streamID](Asset* asset) mutable
		{
			auto* ia = dynamic_cast<IncrementalAsset*>(asset);
			if(ia)
			{
				std::cout<< "Sending header for: " << ia->id << std::endl;
				ia->serializeHeader(ctxPtr->res);

				IncrementalAssetSender assetSender{};
				assetSender.iteratorData = ia->createContext();
				assetSender.asset = ia;
				assetSender.streamID = streamID;
				assetSender.connection = ctxPtr->sender;
				_senders.push_back(std::move(assetSender));
                ctxPtr = nullptr;
			}
			else
				std::cerr << "Tried to request non-incremental asset as incremental" << std::endl;
		};

		if(asset)
			f(asset);
		else
			_am.fetchAsset<Asset>(id).then(f);

	});

	_nm.addRequestListener("saveAsset",[this](auto& rc){
		auto ctx = getContext(rc.sender);
		if(!validatePermissions(ctx, {"edit assets"}))
        {
            rc.code = net::ResponseCode::denied;
            return;
        }
		std::string assetPath;       //desired asset path
        rc.req >> assetPath;

		if(!checkAssetPath(assetPath))
		{
            rc.code = net::ResponseCode::denied;
			return;
		}
        Asset* asset = Asset::deserializeUnknown(rc.req);
		std::string fullPath = fullAssetPath(assetPath + "/" + asset->name + "." + asset->type.toString());


        if(asset->id.serverAddress.empty())
        {
            AssetInfo info{};
            info.name = asset->name;
            info.filename = assetPath + asset->name + "." + asset->type.toString();
            _db.insertAssetInfo(info);
            asset->id.id = info.id;
            asset->id.serverAddress = Config::json()["network"]["domain"].asString();
        }
        _fm.writeAsset(asset, fullPath);
        rc.res << asset->id;
	});

	addDirectoryRequestListeners();
	Runtime::timeline().addTask("send asset data", [this]{processMessages();}, "networking");
}


void AssetServer::addDirectoryRequestListeners()
{
	_nm.addRequestListener("directoryContents", [this](auto& rc){
		auto ctx = getContext(rc.sender);
		if(!validatePermissions(ctx, {"edit assets"}))
        {
            rc.code = net::ResponseCode::denied;
            return;
        }
		std::string dirPath;
		rc.req>> dirPath;
		if(!checkAssetPath(dirPath))
		{
            rc.code = net::ResponseCode::denied;
			return;
		}
		auto contents = _fm.getDirectoryContents(fullAssetPath(dirPath));
        rc.res << contents.directories << static_cast<uint16_t>(contents.files.size());
		for(auto& f : contents.files)
		{
            rc.res << f;
			AssetID id;
			bool isAsset = _db.fileToAssetID(dirPath + f, id);
            rc.res << isAsset;
			if(isAsset)
			{
				id.serverAddress = Config::json()["network"]["domain"].asString();
                rc.res << id;
			}
		}
	});

	_nm.addRequestListener("createDirectory", [this](auto& rc){
		auto ctx = getContext(rc.sender);
		if(!validatePermissions(ctx, {"edit assets"}))
        {
            rc.code = net::ResponseCode::denied;
            return;
        }
		std::string dirPath;
		rc.req>> dirPath;
		if(!checkAssetPath(dirPath))
		{
            rc.code = net::ResponseCode::denied;
			return;
		}
		_fm.createDirectory(fullAssetPath(dirPath));
	});

	_nm.addRequestListener("moveDirectory", [this](auto& rc){
		auto ctx = getContext(rc.sender);
		if(!validatePermissions(ctx, {"edit assets"}))
        {
            rc.code = net::ResponseCode::denied;
            return;
        }
		std::string dirSrc, dirDest;
		rc.req>> dirSrc >> dirDest;
		if(!checkAssetPath(dirSrc) || !checkAssetPath(dirDest))
		{
            rc.code = net::ResponseCode::denied;
			return;
		}
		_fm.moveFile(fullAssetPath(dirSrc), fullAssetPath(dirDest));
        _db.moveAssets(dirSrc, dirDest);
	});

	_nm.addRequestListener("deleteFile", [this](auto& rc){
		auto ctx = getContext(rc.sender);
		if(!validatePermissions(ctx, {"edit assets"}))
        {
            rc.code = net::ResponseCode::denied;
            return;
        }
		std::string dirPath;
		rc.req >> dirPath;
		if(!checkAssetPath(dirPath))
		{
            rc.code = net::ResponseCode::denied;
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
			_fm.deleteFile(fullAssetPath(dirPath));
		rc.res << notRoot;
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
		_fm.async_readUnknownAsset(fullAssetPath(info.filename)).then([this, asset, id](Asset* data){
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

std::string AssetServer::fullAssetPath(const std::string& postfix)
{
    std::string combined = Config::json()["data"]["asset_path"].asString() + "/" + postfix;
    std::string output;
    output.reserve(combined.size());
    char lastChar = '\0';
    for(char c : combined)
    {
        if(c != '/' || lastChar != '/')
            output += c;
        lastChar = c;
    }

    return output;
}



