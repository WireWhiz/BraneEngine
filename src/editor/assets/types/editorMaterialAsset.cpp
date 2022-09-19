//
// Created by eli on 8/25/2022.
//

#include "editorMaterialAsset.h"
#include "assets/types/materialAsset.h"

EditorMaterialAsset::EditorMaterialAsset(const std::filesystem::path& file, BraneProject& project) : EditorAsset(file, project)
{
	// Generate default
	if(!std::filesystem::exists(_file))
	{
		_json.data()["inputs"] = Json::arrayValue;
		_json.data()["vertexShader"] = "null";//TODO default fragment and vertex shaders applied here
		_json.data()["fragmentShader"] = "null";
	}
}

Asset* EditorMaterialAsset::buildAsset(const AssetID& id) const
{
	assert(id.string() == _json["id"].asString());
	auto* material = new MaterialAsset();
	material->name = name();
	material->id = _json["id"].asString();
	material->vertexShader = _json["vertexShader"].asString();
	material->fragmentShader = _json["fragmentShader"].asString();
	return material;
}

std::vector<std::pair<AssetID, AssetType>> EditorMaterialAsset::containedAssets() const
{
	std::vector<std::pair<AssetID, AssetType>> deps;
	deps.emplace_back(AssetID{_json["id"].asString()}, AssetType::material);
	return std::move(deps);
}
