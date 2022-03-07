#pragma once
#include "assets/assetID.h"
#include <networking/serializedData.h>
#include <byte.h>
#include <assets/assetType.h>
//namespace net
//{
//	class OMessage;
//	class IMessage;
//}

class AssetManager;



class Asset
{
public:
	enum LoadState{
		null = 0,
		partial = 1,
		complete = 2
	};
private:
public:

	std::string name;
	AssetID id;
	AssetType type;
	LoadState loadState;
	static Asset* deserializeUnknown(ISerializedData& message, AssetManager& am);
	virtual void serialize(OSerializedData& message);
	virtual void deserialize(ISerializedData& message, AssetManager& am);
};

class IncrementalAsset : public Asset
{

};

template <>
struct std::hash<Asset>
{
	size_t operator()(const Asset& k) const;
};
