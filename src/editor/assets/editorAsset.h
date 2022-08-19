//
// Created by eli on 8/18/2022.
//

#ifndef BRANEENGINE_EDITORASSET_H
#define BRANEENGINE_EDITORASSET_H

#include <filesystem>
#include <utility/jsonVersioner.h>
#include <utility/serializedData.h>

class Asset;
class EditorAsset
{
protected:
	//Editable data goes in the json for easy versioning, other things will be serialized in binary format
	VersionedJson _json;
	std::filesystem::path _file;
	virtual Json::Value defaultJson() const;
	virtual void serialize(OutputSerializer& s);
	virtual void deserialize(InputSerializer& s);
public:
	static std::filesystem::path assetPathFromSource(const std::filesystem::path& path);
	static EditorAsset* openUnknownAsset(const std::filesystem::path& path, JsonVersionTracker& tkr);
	EditorAsset(const std::filesystem::path& file, JsonVersionTracker& tkr);
	virtual ~EditorAsset() = default;
	void load();
	virtual void rebuildAsset(Asset* asset);
	bool unsavedChanged() const;
	void save();
	VersionedJson& json();
};


#endif //BRANEENGINE_EDITORASSET_H
