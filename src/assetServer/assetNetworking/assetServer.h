//
// Created by eli on 3/3/2022.
//

#ifndef BRANEENGINE_ASSETSERVER_H
#define BRANEENGINE_ASSETSERVER_H
#include <networking/networking.h>
#include <assets/assetManager.h>
#include <list>

struct IncrementalAssetSender
{
	std::unique_ptr<IncrementalAsset::SerializationContext> iteratorData;
	IncrementalAsset* asset = nullptr;
	net::Connection* dest = nullptr;
};

class AssetServer
{
	NetworkManager& _nm;
	AssetManager& _am;
	std::mutex _cLock;
	std::vector<std::unique_ptr<net::Connection>> _connections;

	std::list<IncrementalAssetSender> _senders;
public:
	AssetServer(NetworkManager& nm, AssetManager& am);
	~AssetServer();
	void processMessages();
};


#endif //BRANEENGINE_ASSETSERVER_H
