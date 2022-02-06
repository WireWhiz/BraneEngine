#pragma once
#include "assets/assetID.h"
#include <networking/serializedData.h>
#include <byte.h>
//namespace net
//{
//	class OMessage;
//	class IMessage;
//}

enum class AssetDependencyLevel
{
	optional,
	loadProcedural,
	requireFull
};

struct AssetDependency
{
	AssetID id;
	AssetDependencyLevel level;
};

struct AssetHeader
{
	AssetID id;
	std::vector<AssetDependency> dependencies;
	void serialize(OSerializedData& sData);
	AssetHeader() = default;
	AssetHeader(ISerializedData& serializedHeader);
};

class SerializedAssetBody
{
 
};

class SerializedAssetBodyChunk
{

};

class Asset
{
protected:
	AssetHeader _header;

public:
	std::string name;
	const AssetID& id() const;
	AssetID& id();

	AssetHeader* header();
	virtual void serialize(OSerializedData& message);
	virtual void deserialize(ISerializedData& message);
};

template <>
struct std::hash<Asset>
{
	size_t operator()(const Asset& k) const;
};
