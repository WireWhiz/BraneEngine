//
// Created by eli on 8/25/2022.
//

#include "editorMaterialAsset.h"
#include "assets/types/materialAsset.h"

EditorMaterialAsset::EditorMaterialAsset(const std::filesystem::path& file, BraneProject& project) : EditorAsset(file, project)
{

}

Json::Value EditorMaterialAsset::defaultJson()
{
	Json::Value value = EditorAsset::defaultJson();
	value["inputs"] = Json::arrayValue;
	value["vertexShader"] = "null";//TODO default fragment and vertex shaders applied here
	value["fragmentShader"] = "null";
	return value;
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
	return {{_json["id"].asString(), AssetType::material}};
}
