#pragma once

#include "../assetID.h"
#include <memory>
#include <ecs/core/virtualType.h>

class ComponentAsset
{
private:
	AssetID _id;
	size_t _size;
	std::vector<std::shared_ptr<VirtualType>> _types;
public:
	ComponentAsset(const ComponentAsset& source);
	ComponentAsset(const std::vector<std::shared_ptr<VirtualType>>& types, AssetID id);
	~ComponentAsset();
	void setSize(size_t size);
	const AssetID& id() const;
	size_t size() const;
	size_t getByteIndex(size_t index) const;
	const std::vector<std::shared_ptr<VirtualType>>& types() const;
};