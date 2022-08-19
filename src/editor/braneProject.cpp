//
// Created by eli on 8/14/2022.
//

#include "braneProject.h"
#include "runtime/runtime.h"
#include "fileManager/fileManager.h"
#include "fileManager/fileWatcher.h"
#include "assets/editorAsset.h"
#include "assets/types/editorShaderAsset.h"
#include <fstream>

BraneProject::BraneProject(JsonVersionTracker& tkr) : _file(tkr)
{

}

BraneProject::~BraneProject()
{

}

void BraneProject::loadDefault()
{
	Json::Value file;
	try
	{
		if(!FileManager::readFile("defaultAssets/defaultProjectFile.brane", file))
		{
			Runtime::error("Could not open default project file");
			throw std::runtime_error("Could not open default project file");
		}
	}
	catch(const std::exception& e)
	{
		Runtime::error("Error parsing default project file: " + (std::string)e.what());
		throw std::runtime_error("Error  parsing default project file!");
	}
	_file.initialize(file);
}

bool BraneProject::load(const std::filesystem::path& filepath)
{
	_filepath = filepath;
	Json::Value file;
	try
	{
		if(!FileManager::readFile(filepath.string(), file))
		{
			Runtime::error("Could not open " + filepath.string());
			return false;
		}
	}
	catch(const std::exception& e)
	{
		Runtime::error("Error opening " + filepath.string() + ". " + e.what());
		return false;
	}
	_file.initialize(file);
	initLoaded();
	return true;
}

void BraneProject::create(const std::string& projectName, const std::filesystem::path& directory)
{
	loadDefault();
	_file.data()["info"]["name"] = projectName;
	_filepath = directory;
	_filepath = _filepath / projectName / (projectName + ".brane");

	save();
	std::filesystem::create_directory(projectDirectory() / "assets");
	std::filesystem::create_directory(projectDirectory() / "cache");
	initLoaded();
}

void BraneProject::save()
{
	FileManager::writeFile(_filepath.string(), _file.data());
	_file.markClean();
}

bool BraneProject::loaded() const
{
	return _loaded;
}

std::filesystem::path BraneProject::projectDirectory()
{
	return _filepath.parent_path();
}

void BraneProject::initLoaded()
{
	_fileWatcher = std::make_unique<FileWatcher>();
	_fileWatcher->watchDirectory(projectDirectory() / "assets");
	_fileWatcher->addFileWatcher(".gltf", [this](const std::filesystem::path& path){
		Runtime::log("loading gltf: " + path.string());
	});
	_fileWatcher->addFileWatcher(".vert", [this](const std::filesystem::path& path){
		Runtime::log("loading fragment shader: " + path.string());
		std::filesystem::path assetPath = path;
		assetPath.replace_extension(".shader");

		bool isOpen = false;
		EditorShaderAsset* shaderAsset;
		if(_openAssets.count(assetPath.string()))
		{
			shaderAsset = (EditorShaderAsset*)_openAssets.at(assetPath.string()).get();
			isOpen = true;
		}
		else
			shaderAsset = new EditorShaderAsset(assetPath, _file.tracker());

		shaderAsset->updateFromSource(path);

		_idToAssetPath[AssetID(shaderAsset->json().data()["id"].asString())] = assetPath.string();

		if(!isOpen)
			delete shaderAsset;
	});
	_fileWatcher->addFileWatcher(".frag", [this](const std::filesystem::path& path){
		Runtime::log("loading vertex shader: " + path.string());
		std::filesystem::path assetPath = path;
		assetPath.replace_extension(".shader");

		bool isOpen = false;
		EditorShaderAsset* shaderAsset;
		if(_openAssets.count(assetPath.string()))
		{
			shaderAsset = (EditorShaderAsset*)_openAssets.at(assetPath.string()).get();
			isOpen = true;
		}
		else
			shaderAsset = new EditorShaderAsset(assetPath, _file.tracker());

		shaderAsset->updateFromSource(path);

		_idToAssetPath[AssetID(shaderAsset->json().data()["id"].asString())] = assetPath.string();

		if(!isOpen)
			delete shaderAsset;
	});
	_fileWatcher->scanForChanges();
}

bool BraneProject::unsavedChanges() const
{
	return _file.dirty();
}

VersionedJson& BraneProject::json()
{
	return _file;
}

std::shared_ptr<EditorAsset> BraneProject::getEditorAsset(AssetID id)
{
	if(_idToAssetPath.count(id))
		return getEditorAsset(std::filesystem::path(_idToAssetPath.at(id)));
	return nullptr;
}

std::shared_ptr<EditorAsset> BraneProject::getEditorAsset(std::filesystem::path path)
{
	if(_openAssets.count(path.string()))
		return _openAssets.at(path.string());
	auto asset = std::shared_ptr<EditorAsset>(EditorAsset::openUnknownAsset(path, _file.tracker()));
	if(asset)
		_openAssets.insert({path.string(), asset});
	return asset;
}
