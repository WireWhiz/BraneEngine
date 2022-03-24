#include "asset.h"


void Asset::serialize(OSerializedData& sData)
{
	sData << id  << name << type.string();
}

void Asset::deserialize(ISerializedData& sData, AssetManager& am)
{
	std::string typeStr;
	sData >> id >> name >> typeStr;
	type.set(typeStr);
	loadState = LoadState::complete;
}

#include "types/componentAsset.h"
#include "types/meshAsset.h"
#include "assembly.h"
Asset* Asset::deserializeUnknown(ISerializedData& sData, AssetManager& am)
{

	std::string typeStr;
	AssetID id;
	std::string name;
	sData >> id >> name >> typeStr;
	AssetType type;
	type.set(typeStr);
	Asset* asset;
	switch(type.type())
	{
		case AssetType::none:
			throw std::runtime_error("Can't deserialize none type");
			break;
		case AssetType::component:
			asset = new ComponentAsset();
			break;
		case AssetType::system:
			assert("Not implemented");
			break;
		case AssetType::mesh:
			asset = new MeshAsset();
			break;
		case AssetType::texture:
			assert("Not implemented");
			break;
		case AssetType::shader:
			assert("Not implemented");
			break;
		case AssetType::material:
			assert("Not implemented");
			break;
		case AssetType::assembly:
			asset = new Assembly();
			break;
		case AssetType::player:
			assert("Not implemented");
			break;
	}
	sData.restart();
	asset->deserialize(sData, am);
	return asset;
}

void IncrementalAsset::serializeHeader(OSerializedData& sData)
{
	Asset::serialize(sData);
	sData << _incrementCount;
}

void IncrementalAsset::deserializeHeader(ISerializedData& sData, AssetManager& am)
{
	Asset::deserialize(sData, am);
	sData >> _incrementCount;
}

size_t IncrementalAsset::incrementCount() const
{
	return _incrementCount;
}

IncrementalAsset* IncrementalAsset::deserializeUnknownHeader(ISerializedData& sData, AssetManager& am)
{
	std::string typeStr;
	AssetID id;
	std::string name;
	sData >> id >> name >> typeStr;
	AssetType type;
	type.set(typeStr);
	IncrementalAsset* asset;
	switch(type.type())
	{
		case AssetType::none:
			throw std::runtime_error("Can't deserialize none type");
		case AssetType::mesh:
			asset = new MeshAsset();
			break;
		default:
			throw std::runtime_error("Tried to incrementally deserialize, non-incremental asset.");
	}
	sData.restart();
	asset->deserializeHeader(sData, am);
	return asset;
}

bool IncrementalAsset::serializeIncrement(OSerializedData& sData, void*& iteratorData)
{
	sData << id; // We don't have id in the deserialize function because it is consumed by the
				 // networkManager before the call to deserializeIncrement
	return false; //Return false because there is no more data
}
