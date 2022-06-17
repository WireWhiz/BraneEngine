#pragma once

#include "config/config.h"
#include "runtime/module.h"
#include "utility/threadPool.h"
#include "assets/asset.h"
#include "networking/networking.h"
#include "fileManager/fileManager.h"

class AssetManager;
namespace graphics
{
	class VulkanRuntime;
}


class Client : public Module
{
	AssetManager& _am;
	NetworkManager& _nm;

	void addAssetPreprocessors(AssetManager& am, graphics::VulkanRuntime& vkr);
	AsyncData<Asset*> fetchAssetCallback(const AssetID& id, bool incremental);

public:
	Client();

	const char * name() override;
};