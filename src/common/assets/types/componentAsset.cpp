#include "componentAsset.h"
#include "ecs/core/component.h"


ComponentAsset::ComponentAsset()
{
	type.set(AssetType::Type::component);
}

ComponentAsset::ComponentAsset(const std::vector<VirtualType::Type>& members, AssetID id)
{
	this->id = id;
	type.set(AssetType::Type::component);
}

ComponentAsset::~ComponentAsset()
{
}

const std::vector<VirtualType::Type>& ComponentAsset::members() const
{
	return _members;
}

void ComponentAsset::serialize(OSerializedData& sdata)
{
	Asset::serialize(sdata);
	sdata << _members;
}

void ComponentAsset::deserialize(ISerializedData& sdata)
{
	Asset::deserialize(sdata);
	sdata >> _members;
}

void ComponentAsset::toFile(MarkedSerializedData& sData)
{
	Asset::toFile(sData);
	sData.writeAttribute("members", _members);
}

void ComponentAsset::fromFile(MarkedSerializedData& sData)
{
	Asset::fromFile(sData);
	sData.readAttribute("members", _members);
}




