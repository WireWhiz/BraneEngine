#pragma once
#include <stdio.h>
#include <fstream>
#include <json/json.h>
#include <filesystem>
#include <utility/serializedData.h>
#include <utility/asyncData.h>
#include <assets/asset.h>
#include "utility/threadPool.h"
#include <runtime/module.h>


class FileManager : public Module
{
public:
	struct DirectoryContents
	{
		std::vector<std::string> directories;
		std::vector<std::string> files;
	};
	struct Directory
	{
		std::string name;
		bool open = false;
		Directory* parent;
		std::vector<std::unique_ptr<Directory>> children;
		std::filesystem::path path() const;
		void setParentsOpen();
	};

	static DirectoryContents getDirectoryContents(const std::filesystem::path& path);
	static std::unique_ptr<Directory> getDirectoryTree(const std::filesystem::path& path);
	static void createDirectory(const std::filesystem::path& path);
	static bool deleteFile(const std::filesystem::path& path);
	static void moveFile(const std::filesystem::path& source, const std::filesystem::path& destination);

	static std::string requestLocalFilePath(const std::string& title, const std::vector<const char*>& filters);

	FileManager();
	template <typename T>
	static bool readFile(const std::filesystem::path& filename, std::vector<T>& data)
	{
		std::ifstream f(filename, std::ios::binary | std::ios::ate);
		if (!f.is_open())
			return false;

		data.resize(f.tellg() / sizeof(T));
		f.seekg(0);
		f.read((char*)data.data(), data.size() * sizeof(T));
		f.close();

		return true;
	}

	template<typename T>
	T* readAsset(const std::filesystem::path& filename)
	{
		std::ifstream f(filename, std::ios::binary);
		if (!f.is_open())
			return nullptr;
        SerializedData data;
        readFile(filename, data.vector());
		f.close();

        InputSerializer s(data);
		T* asset = new T();
		asset->deserialize(s);
		return asset;
	}

	Asset* readUnknownAsset(const std::filesystem::path& filename)
	{
		std::ifstream f(filename, std::ios::binary);
		if (!f.is_open())
			throw std::runtime_error("File not found!");
        SerializedData data;
        readFile(filename, data.vector());
        f.close();
        InputSerializer s(data);
		return Asset::deserializeUnknown(s);
	}

	template<typename T>
	AsyncData<T*> async_readAsset(const std::filesystem::path& filename)
	{
		AsyncData<T*> asset;
		ThreadPool::enqueue([this, filename, asset]{
			asset.setData(readAsset<T>(filename));
		});
		return asset;
	}

	AsyncData<Asset*> async_readUnknownAsset(const std::filesystem::path& filename);



	static std::string fileHash(const std::filesystem::path& filename);
	static bool readFile(const std::filesystem::path& filename, std::string& data);
	static bool readFile(const std::filesystem::path& filename, Json::Value& data);
	template<typename T>
	void writeFile(const std::filesystem::path& filename, const std::vector<T>& data)
	{
		std::filesystem::path path{filename};
		std::filesystem::create_directories(path.parent_path());



		std::ofstream f(path, std::ios::out | std::ofstream::binary);
		f.write((char*)data.data(), data.size() * sizeof(T));
		f.close();
	}

	static void writeFile(const std::filesystem::path& filename, std::string& data);
	static void writeFile(const std::filesystem::path& filename, Json::Value& data);

	void writeAsset(Asset* asset, const std::filesystem::path& filename);

	static const char* name();
};