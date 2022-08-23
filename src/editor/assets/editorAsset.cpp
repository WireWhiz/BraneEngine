//
// Created by eli on 8/18/2022.
//

#include "editorAsset.h"
#include "fileManager/fileManager.h"
#include <sstream>
#include "assets/asset.h"
#include "types/editorShaderAsset.h"
#include "editor/braneProject.h"
#include "editor/editor.h"
#include "editor/assets/types/editorAssemblyAsset.h"

const std::string endToken = "\n/* Do not edit past this line */\n";

EditorAsset::EditorAsset(const std::filesystem::path& file, BraneProject& project) : _file(file), _project(project), _json(project.editor().jsonTracker())
{
	auto ext = file.extension();
	if(ext == ".shader")
		_type = AssetType::shader;
	else if(ext == ".assembly")
		_type = AssetType::assembly;
	_name = file.stem().string();
	load();
}

bool EditorAsset::unsavedChanged() const
{
	return _json.dirty();
}

void EditorAsset::load()
{
	try{
		if(!FileManager::readFile(_file, _json.data()))
		{
			Runtime::log("Creating " + _file.string());
			_json.data() = defaultJson();
			return;
		}
	} catch(const std::exception& e)
	{
		Runtime::error("Could not parse " + _file.string() + "!\n" + e.what());
		_json.data()["failedLoad"] = true;
	}

}

void EditorAsset::save()
{
	if(_json.data().isMember("failedLoad")){
		Runtime::warn("Tried to save file that did not load correctly! \nAborting to prevent overwrite with further invalid data");
		return;
	}
	FileManager::writeFile(_file, _json.data());
	_json.markClean();
}

VersionedJson& EditorAsset::json()
{
	return _json;
}

Json::Value EditorAsset::defaultJson()
{
	Json::Value value;
	value["id"] = _project.newAssetID(_file).string();
	return value;
}

EditorAsset* EditorAsset::openUnknownAsset(const std::filesystem::path& path, BraneProject& project)
{
	auto ext = path.extension();
	if(ext == ".shader")
		return new EditorShaderAsset(path, project);
	if(ext == ".assembly")
		return new EditorAssemblyAsset(path, project);
	return nullptr;
}

const AssetType& EditorAsset::type() const
{
	return _type;
}

const std::string& EditorAsset::name() const
{
	return _name;
}




