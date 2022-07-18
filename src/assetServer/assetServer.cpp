//
// Created by eli on 3/3/2022.
//

#include "assetServer.h"
#include "common/utility/threadPool.h"
#include "gltf/gltfLoader.h"
#include "assetProcessing/assetBuilder.h"

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

	_nm.addRequestListener("processAsset",[this](net::RequestResponse& res){
		auto ctx = getContext(res.sender());
		if(!validatePermissions(ctx, {"edit assets"}))
			return;
		std::string assetFilename;   //Name of raw file
		std::string assetName;       //desired asset name
		std::string assetPath;       //desired asset path
		std::string assetData; //file data
		res.body() >> assetFilename >> assetName >> assetPath >> assetData;

		if(!checkAssetPath(assetPath))
		{
			res.res() << false;
			res.send();
			return;
		}
		std::string assetDirectory = Config::json()["data"]["asset_path"].asString();

		std::string suffix = assetFilename.substr(assetFilename.find_last_of('.'));
		if(suffix == ".glb")
		{
			gltfLoader loader;
			loader.loadGlbFromString(assetData);
			auto assembly = AssetBuilder::buildAssembly(assetName, loader);

			//Save meshes
			for(auto& mesh : assembly.meshes)
			{
				std::string meshFilename = assetPath + assetName + "_meshes/" + mesh->name + ".mesh";

				AssetInfo info{0, meshFilename, mesh->name, AssetType::mesh};
				_db.insertAssetInfo(info);
				mesh->id.serverAddress = Config::json()["network"]["domain"].asString();
				mesh->id.id = info.id;
				assembly.assembly->meshes.push_back(mesh->id);
				_fm.writeAsset(mesh.get(), assetDirectory + "/" + meshFilename);
			}

			//Save assembly
			std::string assemblyFilename = assetPath + assetName + ".assembly";
			AssetInfo info{0, assemblyFilename, assetName, AssetType::assembly};
			_db.insertAssetInfo(info);
			assembly.assembly->id.serverAddress = Config::json()["network"]["domain"].asString();
			assembly.assembly->id.id = info.id;
			_fm.writeAsset(assembly.assembly.get(), assetDirectory + "/" +assemblyFilename);
		}
		res.res() << true;
		res.send();
	});

	addDirectoryRequestListeners();
	Runtime::timeline().addTask("send asset data", [this]{processMessages();}, "networking");
}


void AssetServer::addDirectoryRequestListeners()
{
	_nm.addRequestListener("directoryContents", [this](net::RequestResponse& res){
		auto ctx = getContext(res.sender());
		if(!validatePermissions(ctx, {"edit assets"}))
			return;
		std::string dirPath;
		res.body() >> dirPath;
		if(!checkAssetPath(dirPath))
		{
			res.res() << false;
			res.send();
			return;
		}
		auto contents = _fm.getDirectoryContents(Config::json()["data"]["asset_path"].asString() + "/" + dirPath);
		res.res() << true << contents.directories << static_cast<uint16_t>(contents.files.size());
		for(auto& f : contents.files)
		{
			res.res() << f;
			AssetID id;
			bool isAsset = _db.fileToAssetID(dirPath + f, id);
			res.res() << isAsset;
			if(isAsset)
			{
				id.serverAddress = Config::json()["network"]["domain"].asString();
				res.res() << id;
			}
		}
		res.send();
	});

	_nm.addRequestListener("createDirectory", [this](net::RequestResponse& res){
		auto ctx = getContext(res.sender());
		if(!validatePermissions(ctx, {"edit assets"}))
			return;
		std::string dirPath;
		res.body() >> dirPath;
		if(!checkAssetPath(dirPath))
		{
			res.res() << false;
			res.send();
			return;
		}
		_fm.createDirectory(Config::json()["data"]["asset_path"].asString() + "/" + dirPath);
		res.res() << true;
		res.send();
	});

	_nm.addRequestListener("moveDirectory", [this](net::RequestResponse& res){
		auto ctx = getContext(res.sender());
		if(!validatePermissions(ctx, {"edit assets"}))
			return;
		std::string dirSrc, dirDest;
		res.body() >> dirSrc >> dirDest;
		if(!checkAssetPath(dirSrc) || !checkAssetPath(dirDest))
		{
			res.res() << false;
			res.send();
			return;
		}
		_fm.moveFile(Config::json()["data"]["asset_path"].asString() + "/" + dirSrc, Config::json()["data"]["asset_path"].asString() + "/" + dirDest);
		res.res() << true;
		res.send();
	});

	_nm.addRequestListener("deleteFile", [this](net::RequestResponse& res){
		auto ctx = getContext(res.sender());
		if(!validatePermissions(ctx, {"edit assets"}))
			return;
		std::string dirPath;
		res.body() >> dirPath;
		if(!checkAssetPath(dirPath))
		{
			res.res() << false;
			res.send();
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
		res.res() << notRoot;
		res.send();
	});
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



