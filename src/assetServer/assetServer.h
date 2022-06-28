//
// Created by eli on 3/3/2022.
//

#ifndef BRANEENGINE_ASSETSERVER_H
#define BRANEENGINE_ASSETSERVER_H
#include "common/networking/networking.h"
#include "common/assets/assetManager.h"
#include "database/database.h"
#include <list>

struct IncrementalAssetSender
{
	std::unique_ptr<IncrementalAsset::SerializationContext> iteratorData;
	IncrementalAsset* asset = nullptr;
	uint32_t streamID;
	net::Connection* connection = nullptr;
};

class AssetServer : public Module
{
	NetworkManager& _nm;
	AssetManager& _am;
	FileManager& _fm;
	Database& _db;

	struct ConnectionContext{
		bool authenticated = false;
		std::string username;
		int64_t userID;
		std::unordered_set<std::string> permissions;
	};

	std::unordered_map<net::Connection*, ConnectionContext> _connectionCtx;
	std::list<IncrementalAssetSender> _senders;
	AsyncData<Asset*> fetchAssetCallback(const AssetID& id, bool incremental);
	ConnectionContext& getContext(net::Connection* connection);
	bool validatePermissions(ConnectionContext& ctx, const std::vector<std::string>& permissions);
public:
	AssetServer();
	~AssetServer();
	void processMessages();

	static const char* name();

};


#endif //BRANEENGINE_ASSETSERVER_H
