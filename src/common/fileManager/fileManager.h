#pragma once
#include <stdio.h>
#include <assets/assetManager.h>

class FileManager
{

public:
	template<typename T>
	T* readAsset(AssetID& id)
	{
		net::IMessage iMessage;
		iMessage.data = readFile<byte>(id, "asset");

		return new T(iMessage);
	}
	
	template <typename T>
	std::vector<T> readFile(AssetID& id, const std::string& fileType)
	{
		std::string path = "assets/" + id.owner + "/" + id.type.string() + "/" + id.name + "." + fileType;

		std::ifstream f(path, std::ios::binary | std::ios::ate);
		if (!f.is_open())
			return std::vector<T>();

		std::vector<T> data(f.tellg() / sizeof(T));
		f.seekg(0);
		f.read((char*)data.data(), data.size() * sizeof(T));
		f.close();

		return data;
	}
	void writeAsset(Asset* asset);
	void writeFile(AssetID& id, const std::string& fileType, const std::vector<byte>& data);
};