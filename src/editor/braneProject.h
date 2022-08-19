//
// Created by eli on 8/14/2022.
//

#ifndef BRANEENGINE_BRANEPROJECT_H
#define BRANEENGINE_BRANEPROJECT_H
#include <string>
#include <json/json.h>
#include <filesystem>
#include <unordered_map>
#include "assets/assetID.h"
#include "utility/jsonVersioner.h"

class FileWatcher;
class EditorAsset;

//This file stores everything to do with our connection to the server
class BraneProject
{
	bool _loaded = false;
	std::filesystem::path _filepath;
	VersionedJson _file;
	std::unique_ptr<FileWatcher> _fileWatcher;
	std::unordered_map<std::string, std::shared_ptr<EditorAsset>> _openAssets;
	std::unordered_map<AssetID, std::string> _idToAssetPath;

	void loadDefault();
	void initLoaded();
public:
	BraneProject(JsonVersionTracker& tkr);
	~BraneProject();
	bool loaded() const;
	bool load(const std::filesystem::path& filepath);
	void create(const std::string& projectName, const std::filesystem::path& directory);
	void save();
	bool unsavedChanges() const;
	std::filesystem::path projectDirectory();
	VersionedJson& json();

	std::shared_ptr<EditorAsset> getEditorAsset(AssetID id);
	std::shared_ptr<EditorAsset> getEditorAsset(std::filesystem::path path);
};


#endif //BRANEENGINE_BRANEPROJECT_H
