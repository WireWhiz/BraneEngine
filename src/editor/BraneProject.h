//
// Created by eli on 8/14/2022.
//

#ifndef BRANEENGINE_BRANEPROJECT_H
#define BRANEENGINE_BRANEPROJECT_H
#include <string>
#include <json/json.h>
#include <filesystem>
#include "assets/assetID.h"

//This file stores everything to do with our connection to the server
class BraneProject
{
	bool _loaded = false;
	std::filesystem::path _filepath;
	Json::Value _file;
	bool _unsavedChanges = false;
	void loadDefault();
public:
	bool loaded() const;
	bool load(const std::filesystem::path& filepath);
	void create(const std::string& projectName, const std::filesystem::path& directory);
	void save();
	bool unsavedChanges() const;
	std::filesystem::path findAsset(const AssetID& id);
	std::filesystem::path projectDirectory();
};


#endif //BRANEENGINE_BRANEPROJECT_H
