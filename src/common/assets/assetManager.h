#pragma once
#include "types/assetTypes.h"
#include "types/asset.h"
#include <unordered_set>
#include <ecs/ecs.h>

class AssetManager
{
	std::unordered_map<AssetID, std::unique_ptr<Asset>> _assets;
	static void downloadAcceptorSystem(EntityManager* em, void* thisPtr);
	NativeForEach _downloadAcceptorFE;

public:
	void startDownloadAcceptorSystem(EntityManager* em);

	void addComponent(ComponentAsset* component);
	ComponentAsset* getComponent(const AssetID& id);
	void addMesh(MeshAsset* mesh);
	MeshAsset* getMesh(const AssetID& id);
};
