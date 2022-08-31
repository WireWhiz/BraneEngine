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
#include "editor/assets/types/editorMaterialAsset.h"

const std::string endToken = "\n/* Do not edit past this line */\n";

EditorAsset::EditorAsset(const std::filesystem::path& file, BraneProject& project) : _file(file), _project(project), _json(project.editor().jsonTracker())
{
	auto ext = file.extension().string();
	if(ext.size() > 1)
		_type.set(ext.substr(1));
	_name = file.stem().string();
	load();
}

bool EditorAsset::unsavedChanged() const
{
	return _json.dirty();
}

bool EditorAsset::load()
{
	try{
		if(!FileManager::readFile(_file, _json.data()))
		{
			Runtime::log("Creating " + _file.string());
			_json.data()["id"] = _project.newAssetID(_file, _type).string();
			return false;
		}
		if(_json.data().get("id", "null").asString() == "null")
			_json.data()["id"] = _project.newAssetID(_file, _type).string();
	} catch(const std::exception& e)
	{
		Runtime::error("Could not parse " + _file.string() + "!\n" + e.what());
		_json.data()["failedLoad"] = true;
		return false;
	}
	return true;


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

EditorAsset* EditorAsset::openUnknownAsset(const std::filesystem::path& path, BraneProject& project)
{
	auto ext = path.extension();
	if(ext == ".shader")
		return new EditorShaderAsset(path, project);
	if(ext == ".material")
		return new EditorMaterialAsset(path, project);
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

const std::filesystem::path& EditorAsset::file() const
{
	return _file;
}




