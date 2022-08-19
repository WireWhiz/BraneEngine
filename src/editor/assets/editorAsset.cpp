//
// Created by eli on 8/18/2022.
//

#include "editorAsset.h"
#include "fileManager/fileManager.h"
#include <sstream>
#include "assets/asset.h"
#include "types/editorShaderAsset.h"

const std::string endToken = "\n/* Do not edit past this line */\n";

EditorAsset::EditorAsset(const std::filesystem::path& file, JsonVersionTracker& tkr) : _file(file), _json(tkr)
{
	load();
}

bool EditorAsset::unsavedChanged() const
{
	return _json.dirty();
}

void EditorAsset::load()
{
	std::string fileData;
	if(!FileManager::readFile(_file, fileData))
	{
		Runtime::log("Creating " + _file.string());
		_json.data() = defaultJson();
		return;
	}
	size_t jsonLength = fileData.find_first_of(endToken);

	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	Json::CharReader* reader = builder.newCharReader();

	Json::Value json;
	std::string error;
	if(!reader->parse(fileData.data(), fileData.data() + jsonLength, &json, &error))
	{
		Runtime::error("Could not parse " + _file.string() + "\n reason: " + error);
		return;
	}
	_json.initialize(json);
	size_t serializedDataStart = jsonLength + endToken.size();
	SerializedData data;
	data.resize(fileData.size() - serializedDataStart);

	std::memcpy(data.data(), fileData.data() + serializedDataStart, fileData.size() - serializedDataStart);
	InputSerializer s(data);
	deserialize(s);
}

void EditorAsset::save()
{
	std::string fileData;

	Json::StreamWriterBuilder builder;
	builder["indentation"] = "\t";
	fileData = Json::writeString(builder, _json.data());
	fileData += endToken;

	SerializedData data;
	OutputSerializer s(data);
	serialize(s);
	fileData += std::string_view((char*)data.data(), data.size());

	FileManager::writeFile(_file, fileData);
	_json.markClean();
}

VersionedJson& EditorAsset::json()
{
	return _json;
}

void EditorAsset::serialize(OutputSerializer& s)
{

}

void EditorAsset::deserialize(InputSerializer& s)
{

}

Json::Value EditorAsset::defaultJson() const
{
	Json::Value value;
	value["name"] = "new asset";
	value["id"] = "null/0";
	return value;
}

void EditorAsset::rebuildAsset(Asset* asset)
{
	asset->name = _json.data()["name"].asString();
	asset->id.parseString(_json.data()["id"].asString());
}

std::filesystem::path EditorAsset::assetPathFromSource(const std::filesystem::path& path)
{
	std::filesystem::path assetPath = path;
	std::string ext = path.extension().string();
	if(ext == ".vert" || ext == ".frag")
		assetPath.replace_extension(".shader");
	else if(ext == ".gltf" || ext == ".glb")
		assetPath.replace_extension(".assembly");
	return assetPath;
}

EditorAsset* EditorAsset::openUnknownAsset(const std::filesystem::path& path, JsonVersionTracker& tkr)
{
	std::string ext = path.extension().string();
	if(ext == ".shader")
		return new EditorShaderAsset(path, tkr);
	return nullptr;
}




