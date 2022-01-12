#pragma once
#include <stdio.h>
#include <assets/assetManager.h>
#include <fstream>
#include <json/json.h>
#include <filesystem>

class FileManager
{
public:
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
	T* readAsset(AssetID& id)
	{
		net::IMessage iMessage;
		if(!readFile(id.path(), iMessage.data))
			return nullptr;

		return new T(iMessage);
	}
	


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
};