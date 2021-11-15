#include "assetManager.h"

void AssetManager::addComponent(ComponentAsset* component)
{
	assert(component->id().type == AssetType::component);
	_assets.emplace(component->id(), component);
}

ComponentAsset* AssetManager::getComponent(const AssetID& id)
{
	assert(id.type == AssetType::component);
	assert(_assets.count(id));
	return (ComponentAsset*)(_assets[id].get());
} 

void AssetManager::addMesh(MeshAsset* mesh)
{
	assert(mesh->id().type == AssetType::mesh);
	_assets[mesh->id()] = std::unique_ptr<Asset>(dynamic_cast<Asset*>(mesh));
}

MeshAsset* AssetManager::getMesh(const AssetID& id)
{
	assert(id.type == AssetType::mesh);
	assert(_assets.count(id));
	return (MeshAsset*)(_assets[id].get());
}


