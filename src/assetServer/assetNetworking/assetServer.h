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
	uint32_t streamID;
	net::Connection* connection = nullptr;
};

class AssetServer : public Module
{
	NetworkManager& _nm;
	AssetManager& _am;

	std::list<IncrementalAssetSender> _senders;
public:
	AssetServer(Runtime& runtime);
	~AssetServer();
	void processMessages();

	const char* name() override;

};


#endif //BRANEENGINE_ASSETSERVER_H
