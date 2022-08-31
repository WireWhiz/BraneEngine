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
#include "assets/assembly.h"
#include "ui/gui.h"
#include "editor/editorEvents.h"
#include "assets/types/materialAsset.h"
#include <regex>

Json::Value EditorAssemblyAsset::defaultJson()
{
	Json::Value json = EditorAsset::defaultJson();
	json["linked"] = false;
	json["dependencies"]["meshes"] = Json::arrayValue;
	json["dependencies"]["materials"] = Json::arrayValue;
	return json;
}

EditorAssemblyAsset::EditorAssemblyAsset(const std::filesystem::path& file, BraneProject& project) : EditorAsset(file, project)
{
	std::regex entityChange(R"((entities/)([0-9]+).*)");
	_json.onChange([this, entityChange](const std::string& path){
		if(std::regex_match(path, entityChange))
		{
			auto index = std::stoi(Json::getPathComponent(path, 1));
			Runtime::getModule<GUI>()->sendEvent(std::make_unique<EntityAssetReloadEvent>(index));
		}
	});
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
		meshData["id"] = _project.newAssetID(_file, AssetType::mesh).string();
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

	Json::ArrayIndex index = 0;
	for(auto& entity : _json.data()["entities"])
	{
		// Set parent value on entities
		if(entity.isMember("children"))
		{
			for(auto& child : entity["children"])
				_json.data()["entities"][child.asUInt()]["parent"] = index;
		}
		index++;
	}

	save();
}

Asset* EditorAssemblyAsset::buildAsset(const AssetID& id) const
{
	if(id.string() == _json["id"].asString())
		return buildAssembly();
	return buildMesh(id);
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

std::vector<std::pair<AssetID, AssetType>> EditorAssemblyAsset::containedAssets() const
{
	std::vector<std::pair<AssetID, AssetType>> assets = {{_json["id"].asString(), AssetType::assembly}};
	if(_json["linked"].asBool())
	{
		for(auto& mesh : _json["linkedMeshes"])
			assets.push_back({mesh["id"].asString(), AssetType::mesh});
	}
	return assets;
}

Asset* EditorAssemblyAsset::buildAssembly() const
{
	auto* assembly = new Assembly();
	assembly->name = name();
	assembly->id = _json["id"].asString();

	for(auto& mesh : _json["dependencies"]["meshes"])
		assembly->meshes.emplace_back(mesh.asString());

	for(auto& material : _json["dependencies"]["materials"])
		assembly->materials.emplace_back(material.asString());

	std::unordered_set<const ComponentDescription*> components;
	for(auto& entity : _json["entities"])
	{
		Assembly::EntityAsset entityAsset;
		if(entity.isMember("name"))
		{
			EntityName name;
			name.name = entity["name"].asString();
			entityAsset.components.emplace_back(name.toVirtual());
		}

		bool hasTransform = false;
		if(entity.isMember("children"))
		{
			Children cc;
			for(auto& c : entity["children"])
				cc.children.push_back({c.asUInt(), 0});
			entityAsset.components.emplace_back(cc.toVirtual());
			hasTransform = true;
		}

		for(auto& comp : entity["components"])
		{
			if(comp["id"] == TRS::def()->asset->id.string())
				hasTransform = true;
			entityAsset.components.push_back(jsonToComponent(comp));
		}
		if(hasTransform)
		{
			Transform transform;
			transform.dirty = true;
			entityAsset.components.emplace_back(transform.toVirtual());
		}

		assembly->entities.push_back(entityAsset);
		for(auto& comp : entityAsset.components)
		{
			if(!components.count(comp.description()))
				components.insert(comp.description());
		}
	}
	for(auto* def : components)
		assembly->components.emplace_back(def->asset->id);

	return assembly;
}

void EditorAssemblyAsset::updateEntity(size_t index, const std::vector<EntityID>& entityMap) const
{
	assert(index < entityMap.size());
	auto em = Runtime::getModule<EntityManager>();
	EntityID id = entityMap[index];

	//Reset entity
	ComponentSet oldComponents = em->getEntityArchetype(id)->components();
	for(ComponentID c: oldComponents)
		if(c != 0) //Don't remove ID
			em->removeComponent(id, c);

	std::vector<VirtualComponent> components;

	const Json::Value& entityData = _json["entities"][(Json::ArrayIndex)index];
	if(entityData.isMember("parent") || entityData.isMember("children"))
		em->addComponent<Transform>(id);
	if(entityData.isMember("parent"))
		Transforms::setParent(id, entityMap[entityData["parent"].asUInt()], *em);

	if(entityData.isMember("children"))
	{
		Children cc;
		for(auto& c : entityData["children"])
			cc.children.push_back(entityMap[c.asUInt()]);
		components.emplace_back(cc.toVirtual());
	}
	for(auto& comp : entityData["components"])
	{
		components.push_back(jsonToComponent(comp));
	}

	for(auto& comp : components)
	{
		em->addComponent(id, comp.description()->id);
		em->setComponent(id, comp);
	}

	if(em->hasComponent<MeshRendererComponent>(id))
	{
		auto* renderer = em->getComponent<MeshRendererComponent>(id);
		auto* am = Runtime::getModule<AssetManager>();
		auto* mesh = am->getAsset<MeshAsset>(_json["dependencies"]["meshes"][renderer->mesh].asString());
		renderer->mesh = mesh->runtimeID;
		for(auto& mID : renderer->materials)
		{
			auto* material = am->getAsset<MaterialAsset>(_json["dependencies"]["materials"][mID].asString());
			mID = material->runtimeID;
		}
	}

}

Asset* EditorAssemblyAsset::buildMesh(const AssetID& id) const
{
	GLTFLoader gltf;
	if(!gltf.loadFromFile(_file.parent_path() / _json["source"].asString()))
	{
		Runtime::error("Could not load " + (_file.parent_path() / _json["source"].asString()).string());
		return nullptr;
	}

	Json::Value meshData = Json::nullValue;
	const Json::Value& meshes = _json["linkedMeshes"];
	for(Json::ArrayIndex index = 0; index < meshes.size(); ++index)
	{
		if(meshes[index]["id"] == id.string())
		{
			meshData = gltf.json()["meshes"][index];
			break;
		}
	}
	if(meshData == Json::nullValue)
		return nullptr;

	auto* mesh = new MeshAsset();
	mesh->name = meshData["name"].asString();
	mesh->id = id;

	for(auto& primitive : meshData["primitives"])
	{
		auto positions = gltf.readVec3Buffer(primitive["attributes"]["POSITION"].asUInt());
		size_t pIndex = mesh->addPrimitive(gltf.readScalarBuffer(primitive["indices"].asUInt()), static_cast<uint32_t>(positions.size()));
		mesh->addAttribute(pIndex, "POSITION", positions);


		if(primitive["attributes"].isMember("NORMAL"))
		{
			auto v = gltf.readVec3Buffer(primitive["attributes"]["NORMAL"].asUInt());
			mesh->addAttribute(pIndex, "NORMAL", v);
		}

		if(primitive["attributes"].isMember("TANGENT"))
		{
			//TODO account for tangents with bitangent sign stored as Vec4
			auto v = gltf.readVec3Buffer(primitive["attributes"]["TANGENT"].asUInt());
			mesh->addAttribute(pIndex, "TANGENT", v);
		}

		//TODO  make it so that we automatically detect all texcoords
		if(primitive["attributes"].isMember("TEXCOORD_0"))
		{
			auto v = gltf.readVec2Buffer(primitive["attributes"]["TEXCOORD_0"].asUInt());
			mesh->addAttribute(pIndex, "TEXCOORD_0", v);
		}

		//TODO: Remove vertices unused by indices array, since primitives reuse buffers

	}
	return mesh;
}
