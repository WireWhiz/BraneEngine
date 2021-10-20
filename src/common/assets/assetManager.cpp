#include "assetManager.h"

AssetManager::AssetContainer::AssetContainer()
{
	_asset = nullptr;
	_destructor = nullptr;
	_instances = std::make_shared<size_t>(1);
}

AssetManager::AssetContainer::~AssetContainer()
{
	if(_destructor && *_instances == 1)
		_destructor(_asset);
	*_instances -= 1;
}

AssetManager::AssetContainer::AssetContainer(const AssetContainer& o)
{
	_instances = o._instances;
	*_instances += 1;
	_asset = o._asset;
	_destructor = o._destructor;
}

void* AssetManager::AssetContainer::asset() const
{
	return _asset;
}

void AssetManager::addComponent(ComponentAsset* component)
{
	assert(component);
	_assets.emplace(component->id(), component);
}

ComponentAsset* AssetManager::getComponent(const AssetID& id)
{
	assert(_assets.count(id));
	return static_cast<ComponentAsset*>(_assets[id].asset());
}

void AssetManager::addMesh(MeshAsset* mesh)
{
	assert(mesh);
	_assets[mesh->id] = AssetContainer(mesh);
}

MeshAsset* AssetManager::getMesh(const AssetID& id)
{
	assert(_assets.count(id));
	return static_cast<MeshAsset*>(_assets[id].asset());
}



