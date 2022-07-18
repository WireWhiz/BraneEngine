#include "componentAsset.h"
#include "ecs/core/component.h"


ComponentAsset::ComponentAsset()
{
	type.set(AssetType::Type::component);
}

ComponentAsset::ComponentAsset(const std::vector<VirtualType::Type>& members, const std::vector<std::string>& memberNames, AssetID id)
{
	this->id = id;
	type.set(AssetType::Type::component);
	_members = members;
	_memberNames = memberNames;
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
	sdata << _members << _memberNames;
}

void ComponentAsset::deserialize(ISerializedData& sdata)
{
	Asset::deserialize(sdata);
	sdata >> _members >> _memberNames;
}

void ComponentAsset::toFile(MarkedSerializedData& sData)
{
	Asset::toFile(sData);
	sData.writeAttribute("members", _members);
	sData.writeAttribute("memberNames", _memberNames);
}

void ComponentAsset::fromFile(MarkedSerializedData& sData)
{
	Asset::fromFile(sData);
	sData.readAttribute("members", _members);
	sData.readAttribute("memberNames", _memberNames);
}

const std::vector<std::string>& ComponentAsset::memberNames() const
{
	return _memberNames;
}

std::vector<VirtualType::Type>& ComponentAsset::members()
{
	return _members;
}

std::vector<std::string>& ComponentAsset::memberNames()
{
	return _memberNames;
}




