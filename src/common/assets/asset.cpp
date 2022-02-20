#include "asset.h"

const AssetID& Asset::id() const
{
	return _header.id;
}

AssetID& Asset::id()
{
	return _header.id;
}

void Asset::serialize(OSerializedData& message)
{
	_header.serialize(message);
}

void Asset::deserialize(ISerializedData& message)
{
	_header = AssetHeader(message);

}

size_t std::hash<Asset>::operator()(const Asset& k) const
{
	return std::hash<AssetID>()(k.id());
}

void AssetHeader::serialize(OSerializedData& sData)
{
	sData << id;
	sData << (uint32_t)dependencies.size();
	for (AssetDependency& dep : dependencies)
	{
		sData << dep.id;
		sData << (uint8_t)dep.level;
	}
}

AssetHeader::AssetHeader(ISerializedData& sData)
{
	sData >> id;
	uint32_t size;
	sData >> size;
	dependencies.resize(size);
	for (uint32_t i = 0; i < size; ++i)
	{
		sData >> dependencies[i].id;
		uint8_t level;
		sData >> level;
		dependencies[i].level = (AssetDependency::Level)level;
	}
}
