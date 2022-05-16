#include "fileManager.h"
#include <fstream>
#include <utility/serializedData.h>

void FileManager::writeAsset(Asset* asset)
{
	MarkedSerializedData data;
	asset->toFile(data);

	std::string filename = asset->id.path();
	std::filesystem::path path{filename};
	std::filesystem::create_directories(path.parent_path());

	std::ofstream f(path, std::ios::out | std::ofstream::binary);
	data.writeToFile(f);
	f.close();
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

AsyncData<Asset*> FileManager::async_readUnknownAsset(const AssetID& id, AssetManager& am)
{
	AsyncData<Asset*> asset;
	ThreadPool::enqueue([this, &id, &am, asset]{
		asset.setData(readUnknownAsset(id, am));
	});
	return asset;
}

const char* FileManager::name()
{
	return "fileManager";
}

FileManager::FileManager(Runtime& runtime) : Module(runtime)
{

}
