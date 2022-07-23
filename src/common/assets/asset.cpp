#include "asset.h"


void Asset::serialize(OutputSerializer s)
{
	s << id << name << type.toString();
}

void Asset::deserialize(InputSerializer s)
{
	std::string typeStr;
	s >> id >> name >> typeStr;
	type.set(typeStr);
	loadState = LoadState::complete;
}

#include "types/componentAsset.h"
#include "types/meshAsset.h"
#include "assembly.h"

Asset* Asset::assetFromType(AssetType type)
{
	switch(type.type())
	{
		case AssetType::none:
			throw std::runtime_error("Can't deserialize none type");
			break;
		case AssetType::component:
			return new ComponentAsset();
		case AssetType::system:
			assert("Not implemented");
			break;
		case AssetType::mesh:
			return new MeshAsset();
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
			return new Assembly();
			break;
		case AssetType::player:
			assert("Not implemented");
			break;
	}
	return nullptr;
}

Asset* Asset::deserializeUnknown(InputSerializer s)
{

	std::string typeStr;
	AssetID id;
	std::string name;
	s >> id >> name >> typeStr;
	AssetType type;
	type.set(typeStr);
	Asset* asset = assetFromType(type);
	s.restart();
	if(!asset)
		return nullptr;
	asset->deserialize(s);
	return asset;
}

void IncrementalAsset::serializeHeader(OutputSerializer s)
{
	Asset::serialize(s);
}

void IncrementalAsset::deserializeHeader(InputSerializer s)
{
	Asset::deserialize(s);
}

IncrementalAsset* IncrementalAsset::deserializeUnknownHeader(InputSerializer s)
{
	std::string typeStr;
	AssetID id;
	std::string name;
	s >> id >> name >> typeStr;
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
	s.restart();
	asset->deserializeHeader(s);
	return asset;
}

bool IncrementalAsset::serializeIncrement(OutputSerializer s, SerializationContext* iteratorData)
{
	return false; //Return false because there is no more data
}
