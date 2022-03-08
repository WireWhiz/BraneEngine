#include "asset.h"


void Asset::serialize(OSerializedData& message)
{
	message << id  << name << type.string();
}

void Asset::deserialize(ISerializedData& message, AssetManager& am)
{
	std::string typeStr;
	message >> id >> name >> typeStr;
	type.set(typeStr);
	loadState = LoadState::complete;
}

#include "types/componentAsset.h"
#include "types/meshAsset.h"
#include "assembly.h"
Asset* Asset::deserializeUnknown(ISerializedData& message, AssetManager& am)
{
	std::string typeStr;
	AssetID id;
	std::string name;
	message >> id >> name >> typeStr;
	AssetType type;
	type.set(typeStr);
	Asset* asset;
	switch(type.type())
	{
		case AssetType::none:
			assert("Can't deserialize none");
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
	message.restart();
	asset->deserialize(message, am);
	return asset;
}

size_t std::hash<Asset>::operator()(const Asset& k) const
{
	return std::hash<AssetID>()(k.id);
}

