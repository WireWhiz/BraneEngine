#pragma once
#include "types/assetTypes.h"
#include "types/asset.h"
#include <unordered_set>


class AssetManager
{
	std::unordered_map<AssetID, std::unique_ptr<Asset>> _assets;
public:
	void addComponent(ComponentAsset* component);
	ComponentAsset* getComponent(const AssetID& id);
	void addMesh(MeshAsset* mesh);
	MeshAsset* getMesh(const AssetID& id);
};
