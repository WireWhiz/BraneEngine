#pragma once

#include "../assetID.h"
#include <memory>
#include <ecs/core/virtualType.h>
#include <vector>

class ComponentAsset
{
private:
	AssetID _id;
	size_t _size;
	std::vector<std::unique_ptr<VirtualType>> _types;
public:
	ComponentAsset(const ComponentAsset&) = delete;
	ComponentAsset(std::vector<std::unique_ptr<VirtualType>>& types, AssetID id);
	ComponentAsset(std::vector<VirtualType*>& types, AssetID id, size_t size);
	~ComponentAsset();
	const AssetID& id() const;
	size_t size() const;
	size_t getByteIndex(size_t index) const;
	const std::vector<std::unique_ptr<VirtualType>>& types() const;

	void construct(byte* component) const;
	void deconstruct(byte* component) const;
	void copy(byte* dest, byte* source) const;
	void move(byte* dest, byte* source) const;
};