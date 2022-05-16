#pragma once
#include <stdio.h>
#include <fstream>
#include <json/json.h>
#include <filesystem>
#include <utility/serializedData.h>
#include <assets/asset.h>
#include "utility/threadPool.h"
#include <runtime/module.h>

class FileManager : public Module
{
public:
	explicit FileManager(Runtime& runtime);
	template <typename T>
	bool readFile(const std::string& filename, std::vector<T>& data)
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
	T* readAsset(const AssetID& id, AssetManager& am)
	{
		std::ifstream f(id.path(), std::ios::binary);
		if (!f.is_open())
			return nullptr;
		MarkedSerializedData sData(f);
		f.close();
		T* asset = new T();
		asset->fromFile(sData, am);
		return asset;
	}

	Asset* readUnknownAsset(const AssetID& id, AssetManager& am)
	{
		std::ifstream f(id.path(), std::ios::binary);
		if (!f.is_open())
			return nullptr;
		MarkedSerializedData sData(f);
		f.close();
		return Asset::readUnknown(sData, am);
	}

	template<typename T>
	AsyncData<T*> async_readAsset(const AssetID& id, AssetManager& am)
	{
		AsyncData<T*> asset;
		ThreadPool::enqueue([this, &id, &am, asset]{
			asset.setData(readAsset<T>(id, am));
		});
		return asset;
	}

	AsyncData<Asset*> async_readUnknownAsset(const AssetID& id, AssetManager& am);



	bool readFile(const std::string& filename, std::string& data);
	bool readFile(const std::string& filename, Json::Value& data);
	template<typename T>
	void writeFile(const std::string& filename, const std::vector<T>& data)
	{
		std::filesystem::path path{filename};
		std::filesystem::create_directories(path.parent_path());



		std::ofstream f(path, std::ios::out | std::ofstream::binary);
		f.write((char*)data.data(), data.size() * sizeof(T));
		f.close();
	}

	void writeFile(const std::string& filename, std::string& data);
	void writeFile(const std::string& filename, Json::Value& data);

	void writeAsset(Asset* asset);

	const char* name() override;
};