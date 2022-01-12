#include "fileManager.h"
#include <fstream>
#include <networking/message.h>

void FileManager::writeAsset(Asset* asset)
{
	AssetID& id = asset->id();
	
	net::OMessage oMessage;
	asset->serialize(oMessage);

	writeFile(asset->id().path(), oMessage.data);
}



bool FileManager::readFile(const std::string& filename, std::string& data)
{
	std::ifstream f(filename, std::ios::binary | std::ios::ate);
	if (!f.is_open())
		return false;

	data.resize(f.tellg());
	f.seekg(0);
	f.read((char*)data.data(), data.size());
	f.close();

	return true;
}

bool FileManager::readFile(const std::string& filename, Json::Value& data)
{
	std::ifstream f(filename, std::ios::binary);
	if (!f.is_open())
		return false;

	f >> data;
	f.close();
	return true;
}

void FileManager::writeFile(const std::string& filename, std::string& data)
{
	std::filesystem::path path{filename};
	std::filesystem::create_directories(path.parent_path());

	std::ofstream f(path, std::ios::out | std::ofstream::binary);
	f.write((char*)data.data(), data.size());
	f.close();
}

void FileManager::writeFile(const std::string& filename, Json::Value& data)
{
	std::filesystem::path path{filename};
	std::filesystem::create_directories(path.parent_path());

	std::ofstream f(path, std::ios::out | std::ofstream::binary);
	f << data;
	f.close();
}
