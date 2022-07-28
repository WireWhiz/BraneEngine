#pragma once
#include "assets/assetID.h"
#include <byte.h>
#include <assets/assetType.h>
#include <memory>

class AssetManager;
class InputSerializer;
class OutputSerializer;

class Asset
{
public:
	enum LoadState{
		null = 0,
		partial = 1,
		complete = 2
	};
private:
	static Asset* assetFromType(AssetType type);
public:

	std::string name;
	AssetID id;
	AssetType type;
	LoadState loadState;
	static Asset* deserializeUnknown(InputSerializer& s);
	virtual void serialize(OutputSerializer& s);
	virtual void deserialize(InputSerializer& s);
};

class IncrementalAsset : public Asset
{
public:
    struct SerializationContext{};
	static IncrementalAsset* deserializeUnknownHeader(InputSerializer& s);
	virtual void serializeHeader(OutputSerializer& s);
	virtual void deserializeHeader(InputSerializer& s);
	virtual bool serializeIncrement(OutputSerializer& s, SerializationContext* iteratorData);
	virtual void deserializeIncrement(InputSerializer& s) = 0;
	virtual std::unique_ptr<SerializationContext> createContext() const = 0;
};


