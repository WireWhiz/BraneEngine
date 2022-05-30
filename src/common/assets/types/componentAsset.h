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
	ComponentAsset();
	ComponentAsset(const ComponentAsset&) = delete;
	ComponentAsset(std::vector<std::unique_ptr<VirtualType>>& types, AssetID id);
	ComponentAsset(const std::vector<VirtualType*>& types, AssetID id, size_t size);
	~ComponentAsset();
	size_t size() const;
	size_t getByteIndex(size_t index) const;
	const std::vector<std::unique_ptr<VirtualType>>& types() const;
	void serializeComponent(OSerializedData& sdata, byte* component) const;
	void deserializeComponent(ISerializedData& sdata, byte* component) const;

	//void toFile(MarkedSerializedData& sData) override;
	//void fromFile(MarkedSerializedData& sData, AssetManager& am) override;
	void serialize(OSerializedData& sdata) override;
	void deserialize(ISerializedData& sdata, AssetManager& am) override;
	Json::Value toJson(byte* component) const;
	void fromJson(Json::Value& json, byte* component) const;

	void construct(byte* component) const;
	void deconstruct(byte* component) const;
	void copy(byte* dest, byte* source) const;
	void move(byte* dest, byte* source) const;
};