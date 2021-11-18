#include "fileManager.h"
#include <fstream>
#include <networking/message.h>
#include <filesystem>

Asset* FileManager::readAsset(AssetID id)
{
	std::string path = "assets/" + id.owner + "/" + id.type.string() + "/" + id.name + ".asset";
	net::IMessage iMessage;
	std::ifstream f(path, std::ios::binary | std::ios::ate);
	if (!f.is_open())
		return nullptr;
	iMessage.data.resize(f.tellg());
	f.seekg(0);
	f.read((char*)iMessage.data.data(), iMessage.data.size());
	f.close();

	Asset* asset = nullptr;
	switch (id.type.type())
	{
		case AssetType::mesh:
		{
			asset = new MeshAsset(iMessage);
		}
			break;
		default:
			assert(false && "Unknown asset type");
			break;
	}

	return asset;
}

void FileManager::writeAsset(Asset* asset)
{
	AssetID& id = asset->id();
	std::string path = "assets/" + id.owner + "/" + id.type.string() + "/" + id.name + ".asset";
	net::OMessage oMessage;
	asset->serialize(oMessage);

	std::filesystem::create_directory("assets");
	std::filesystem::create_directory("assets/" + id.owner);
	std::filesystem::create_directory("assets/" + id.owner + "/" + id.type.string());
	
	std::ofstream f(path, std::ios::out | std::ofstream::binary);
	f.write((char*)oMessage.data.data(), oMessage.size());
	f.close();
}
