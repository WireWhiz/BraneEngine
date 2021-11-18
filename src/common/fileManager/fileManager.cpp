#include "fileManager.h"
#include <fstream>
#include <networking/message.h>
#include <filesystem>

void FileManager::writeAsset(Asset* asset)
{
	AssetID& id = asset->id();
	
	net::OMessage oMessage;
	asset->serialize(oMessage);

	writeFile(asset->id(), "asset", oMessage.data);
}

void FileManager::writeFile(AssetID& id, const std::string& fileType, const std::vector<byte>& data)
{
	std::filesystem::create_directory("assets");
	std::filesystem::create_directory("assets/" + id.owner);
	std::filesystem::create_directory("assets/" + id.owner + "/" + id.type.string());

	std::string path = "assets/" + id.owner + "/" + id.type.string() + "/" + id.name + "." + fileType;

	

	std::ofstream f(path, std::ios::out | std::ofstream::binary);
	f.write((char*)data.data(), data.size());
	f.close();
}
