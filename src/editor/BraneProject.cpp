//
// Created by eli on 8/14/2022.
//

#include "BraneProject.h"
#include "runtime/runtime.h"
#include "fileManager/fileManager.h"
#include <fstream>

void BraneProject::loadDefault()
{
	try
	{
		if(!FileManager::readFile("defaultAssets/defaultProjectFile.brane", _file))
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
}

bool BraneProject::load(const std::filesystem::path& filepath)
{
	_filepath = filepath;
	try
	{
		if(!FileManager::readFile(filepath.string(), _file))
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

	return true;
}

void BraneProject::create(const std::string& projectName, const std::filesystem::path& directory)
{
	loadDefault();
	_file["info"]["name"] = projectName;
	_filepath = directory;
	_filepath = _filepath / projectName / (projectName + ".brane");

	save();
	std::filesystem::create_directory(projectDirectory().append("assets"));
	std::filesystem::create_directory(projectDirectory().append("cache"));
}

std::filesystem::path BraneProject::findAsset(const AssetID& id)
{
	return std::filesystem::path();
}

void BraneProject::save()
{
	FileManager::writeFile(_filepath.string(), _file);
	_unsavedChanges = false;
}

bool BraneProject::loaded() const
{
	return _loaded;
}

std::filesystem::path BraneProject::projectDirectory()
{
	return _filepath.parent_path();
}


