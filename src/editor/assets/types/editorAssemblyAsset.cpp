//
// Created by eli on 8/20/2022.
//

#include "editorAssemblyAsset.h"
#include "fileManager/fileManager.h"
#include "editor/braneProject.h"
#include "../gltfLoader.h"
#include "assets/assetManager.h"
#include "systems/transforms.h"
#include "../jsonVirtualType.h"
#include "utility/jsonTypeUtilities.h"
#include "ecs/nativeTypes/meshRenderer.h"

Json::Value EditorAssemblyAsset::defaultJson()
{
	Json::Value json = EditorAsset::defaultJson();
	json["type"] = "assembly";
	json["linked"] = false;
	return json;
}

EditorAssemblyAsset::EditorAssemblyAsset(const std::filesystem::path& file, BraneProject& project) : EditorAsset(file, project)
{

}

void EditorAssemblyAsset::linkToGLTF(const std::filesystem::path& file)
{
	//To avoid issues of overwriting user changes, only we extract entities once. Users will have to force this by deleting the file manually
	if(_json["linked"].asBool())
		return;

	_json.data()["linked"] = true;
	_json.data()["source"] = (std::filesystem::relative(file, _file.parent_path())).string();

	GLTFLoader gltf;
	gltf.loadFromFile(file);

	for(auto& mesh : gltf.json()["meshes"])
	{
		Json::Value meshData;
		meshData["id"] = _project.newAssetID(_file).string();
		meshData["name"] = mesh["name"];
		_json.data()["linkedMeshes"].append(meshData);
		_json.data()["dependencies"]["meshes"].append(meshData["id"]);
	}

	for(auto& node : gltf.json()["nodes"])
	{
		Json::Value entity;
		if(node.isMember("name"))
			entity["name"] = node["name"];
		TRS trs;
		if(node.isMember("matrix"))
		{
			Transform t;
			t.value = fromJson<glm::mat4>(node["matrix"]);
			entity["components"].append(componentToJson(t));
			trs.fromMat(t.value);
		}
		if(node.isMember("translation"))
			trs.translation = fromJson<glm::vec3>(node["translation"]);
		if(node.isMember("rotation"))
			trs.rotation = fromJson<glm::quat>(node["rotation"]);
		if(node.isMember("scale"))
			trs.scale = fromJson<glm::vec3>(node["scale"]);
		entity["components"].append(componentToJson(trs));

		if(node.isMember("mesh"))
		{
			MeshRendererComponent renderer;
			renderer.mesh = node["mesh"].asUInt();
			for(auto& primitive : gltf.json()["meshes"][renderer.mesh]["primitives"])
				renderer.materials.push_back(primitive["material"].asUInt());
			entity["components"].append(componentToJson(renderer));
		}
		if(node.isMember("children"))
			entity["children"] = node["children"];
		_json.data()["entities"].append(entity);
	}

	_json.data()["rootEntity"] = gltf.json()["scenes"][0]["nodes"].get(Json::ArrayIndex(0), "0");

	save();
}

void EditorAssemblyAsset::cacheAsset()
{
	if(_json["linked"].asBool())
	{
		cacheFromGLTF();
		return;
	}
}

void EditorAssemblyAsset::cacheFromGLTF()
{

}

Json::Value EditorAssemblyAsset::componentToJson(VirtualComponentView component)
{
	Json::Value output;
	output["name"] = component.description()->name;
	output["id"] = component.description()->asset->id.string();

	auto& members = component.description()->asset->members();
	auto& names = component.description()->asset->memberNames();
	for(size_t i = 0; i < members.size(); ++i)
	{
		Json::Value member;
		member["name"] = names[i];
		member["value"] = JsonVirtualType::fromVirtual(component.getVar<byte>(i), members[i]);
		member["type"] = VirtualType::typeToString(members[i]);
		output["members"].append(member);
	}
	return output;
}

VirtualComponent EditorAssemblyAsset::jsonToComponent(Json::Value component)
{
	auto* am = Runtime::getModule<AssetManager>();
	auto* em = Runtime::getModule<EntityManager>();

	auto compID = am->getAsset<ComponentAsset>(component["id"].asString())->componentID;
	auto* description = em->components().getComponentDef(compID);

	VirtualComponent output(description);

	auto& members = description->asset->members();
	auto& names = description->asset->memberNames();
	for(Json::ArrayIndex i = 0; i < members.size(); ++i)
		JsonVirtualType::toVirtual(output.getVar<byte>(i), component["members"][i]["value"], members[i]);

	return output;
}
