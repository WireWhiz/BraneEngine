#pragma once
#include <memory>
#include <ecs/core/virtualType.h>
#include <vector>
#include "../asset.h"

class ComponentAsset : public Asset
{
private:
	size_t _size;
	std::vector<std::unique_ptr<VirtualType>> _types;
public:
	ComponentAsset(const ComponentAsset&) = delete;
	ComponentAsset(ISerializedData& sData);
	ComponentAsset(std::vector<std::unique_ptr<VirtualType>>& types, AssetID id);
	ComponentAsset(std::vector<VirtualType*>& types, AssetID id, size_t size);
	~ComponentAsset();
	size_t size() const;
	size_t getByteIndex(size_t index) const;
	const std::vector<std::unique_ptr<VirtualType>>& types() const;
	void serializeComponent(OSerializedData& sdata, byte* component) const;
	void deserializeComponent(ISerializedData& sdata, byte* component) const;
	void serialize(OSerializedData& sdata) override;
	void deserialize(ISerializedData& sdata) override;

	void construct(byte* component) const;
	void deconstruct(byte* component) const;
	void copy(byte* dest, byte* source) const;
	void move(byte* dest, byte* source) const;
};