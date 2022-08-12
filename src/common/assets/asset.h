#pragma once
#include "assets/assetID.h"
#include <byte.h>
#include <assets/assetType.h>
#include <memory>
#include <atomic>
#include <vector>

class AssetManager;
class InputSerializer;
class OutputSerializer;

struct AssetDependency
{
    AssetID id;
    bool streamable = false;
};

class Asset
{
	static Asset* assetFromType(AssetType type);
public:
	std::string name;
    AssetID id;
    AssetType type;

    virtual ~Asset() = default;
    static Asset* deserializeUnknown(InputSerializer& s);
	virtual void serialize(OutputSerializer& s);
	virtual void deserialize(InputSerializer& s);
    virtual std::vector<AssetDependency> dependencies() const;
    virtual void onDependenciesLoaded();
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
    virtual void onFullyLoaded();
};


