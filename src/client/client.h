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
	class SceneRenderer;
}


class Client : public Module
{
	robin_hood::unordered_map<HashedAssetID, EntityID> _chunkRoots;
	graphics::SceneRenderer* _renderer;
public:
	Client();
	void start() override;
	static const char* name();
};