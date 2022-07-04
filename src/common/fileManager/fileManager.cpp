#include "fileManager.h"
#include <fstream>
#include <utility/serializedData.h>
#include <config/config.h>

void FileManager::writeAsset(Asset* asset, const std::string& filename)
{
	MarkedSerializedData data;
	asset->toFile(data);

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

AsyncData<Asset*> FileManager::async_readUnknownAsset(const std::string& filename)
{
	AsyncData<Asset*> asset;
	ThreadPool::enqueue([this, filename, asset]{
		asset.setData(readUnknownAsset(filename));
	});
	return asset;
}

const char* FileManager::name()
{
	return "fileManager";
}

FileManager::FileManager()
{

}

FileManager::DirectoryContents FileManager::getDirectoryContents(const std::string& p)
{
	std::filesystem::path path{p};
	DirectoryContents contents;
	for(auto& file : std::filesystem::directory_iterator(path))
	{
		if(file.is_directory())
			contents.directories.push_back(file.path().filename().string());
		if(file.is_regular_file())
			contents.files.push_back(file.path().filename().string());
	}
	std::sort(contents.directories.begin(), contents.directories.end());
	std::sort(contents.files.begin(), contents.files.end());

	return contents;
}

void FileManager::createDirectory(const std::string& path)
{
	std::filesystem::create_directories(path);
}

bool FileManager::deleteFile(const std::string& path)
{
	return std::filesystem::remove_all(path);
}

void FileManager::moveFile(const std::string& source, const std::string& destination)
{
	std::filesystem::rename(source, destination);
}
