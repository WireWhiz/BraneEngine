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
	static Asset* deserializeUnknown(ISerializedData& sData, AssetManager& am);
	virtual void serialize(OSerializedData& sData);
	virtual void deserialize(ISerializedData& sData, AssetManager& am);
};

class IncrementalAsset : public Asset
{
protected:
	size_t _incrementCount;
public:
	static IncrementalAsset* deserializeUnknownHeader(ISerializedData& sData, AssetManager& am);
	virtual void serializeHeader(OSerializedData& sData);
	virtual void deserializeHeader(ISerializedData& sData, AssetManager& am);
	virtual bool serializeIncrement(OSerializedData& sData, void*& iteratorData);
	virtual void deserializeIncrement(ISerializedData& sData) = 0;
	size_t incrementCount() const;
};


