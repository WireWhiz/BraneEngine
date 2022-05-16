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

Asset* Asset::deserializeUnknown(ISerializedData& sData, AssetManager& am)
{

	std::string typeStr;
	AssetID id;
	std::string name;
	sData >> id >> name >> typeStr;
	AssetType type;
	type.set(typeStr);
	Asset* asset = assetFromType(type);
	sData.restart();
	if(!asset)
		return nullptr;
	asset->deserialize(sData, am);
	return asset;
}

Asset* Asset::readUnknown(MarkedSerializedData& sData, AssetManager& am)
{
	std::string stype;
	sData.readAttribute("type", stype);
	AssetType type;
	type.set(stype);
	Asset* asset = assetFromType(type);
	if(!asset)
		return nullptr;
	asset->fromFile(sData, am);

	return asset;
}

void Asset::toFile(MarkedSerializedData& sData)
{
	sData.writeAttribute("id", id.string());
	sData.writeAttribute("name", name);
	sData.writeAttribute("type", type.string());
}

void Asset::fromFile(MarkedSerializedData& sData, AssetManager& am)
{
	std::string sid, stype;
	sData.readAttribute("id", sid);
	sData.readAttribute("name", name);
	sData.readAttribute("type", stype);
	id.parseString(sid);
	type.set(stype);
}

void IncrementalAsset::serializeHeader(OSerializedData& sData)
{
	Asset::serialize(sData);
}

void IncrementalAsset::deserializeHeader(ISerializedData& sData, AssetManager& am)
{
	Asset::deserialize(sData, am);
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

bool IncrementalAsset::serializeIncrement(OSerializedData& sData, SerializationContext* iteratorData)
{
	return false; //Return false because there is no more data
}
