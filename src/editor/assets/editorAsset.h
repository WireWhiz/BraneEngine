//
// Created by eli on 8/18/2022.
//

#ifndef BRANEENGINE_EDITORASSET_H
#define BRANEENGINE_EDITORASSET_H

#include <filesystem>
#include <utility/jsonVersioner.h>
#include <utility/serializedData.h>
#include <assets/assetType.h>

class Asset;
class BraneProject;
class EditorAsset
{
protected:
	//Editable data goes in the json for easy versioning, other things will be serialized in binary format
	BraneProject& _project;
	VersionedJson _json;
	std::filesystem::path _file;
	virtual Json::Value defaultJson();
	AssetType _type;
	std::string _name;
public:
	static EditorAsset* openUnknownAsset(const std::filesystem::path& path, BraneProject& project);
	EditorAsset(const std::filesystem::path& file, BraneProject& project);
	virtual ~EditorAsset() = default;
	void load();
	virtual std::vector<std::pair<AssetID, AssetType>> containedAssets() const = 0;
	virtual Asset* buildAsset(const AssetID& id) const = 0;
	const AssetType& type() const;
	const std::string& name() const;
	const std::filesystem::path& file() const;
	bool unsavedChanged() const;
	void save();
	VersionedJson& json();
};


#endif //BRANEENGINE_EDITORASSET_H
