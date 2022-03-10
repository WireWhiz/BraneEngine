//
// Created by eli on 3/3/2022.
//

#ifndef BRANEENGINE_ASSETSERVER_H
#define BRANEENGINE_ASSETSERVER_H
#include <networking/networking.h>
#include <assets/assetManager.h>

class AssetServer
{
	NetworkManager& _nm;
	AssetManager& _am;
	std::mutex _cLock;
	std::vector<std::unique_ptr<net::Connection>> _connections;
public:
	AssetServer(NetworkManager& nm, AssetManager& am);
	~AssetServer();
	void processMessages();
};


#endif //BRANEENGINE_ASSETSERVER_H
