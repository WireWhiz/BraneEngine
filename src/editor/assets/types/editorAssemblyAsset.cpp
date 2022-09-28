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
#include "editor/assets/assemblyReloadManager.h"
#include <regex>

EditorAssemblyAsset::EditorAssemblyAsset(const std::filesystem::path& file, BraneProject& project) : EditorAsset(file, project)
{
	// Generate default
	if(!std::filesystem::exists(_file))
	{
		_json.data()["linked"] = false;
		_json.data()["dependencies"]["meshes"] = Json::arrayValue;
		_json.data()["dependencies"]["materials"] = Json::arrayValue;
		_json.data()["dependencies"]["components"].append(EntityName::def()->asset->id.string());
		Json::Value rootEntity;
		rootEntity["name"] = "root";
		rootEntity["components"] = Json::arrayValue;
		_json.data()["entities"].append(rootEntity);
	}

	std::regex entityComponentChange(R"((entities/)([0-9]+)(/components).*)");
	std::regex entityParentChange(R"((entities/)([0-9]+)(/parent).*)");
	_json.onChange([this, entityComponentChange, entityParentChange](const std::string& path, JsonChangeBase* change, bool undo){
		auto* am = Runtime::getModule<AssetManager>();
		auto* assembly = am->getAsset<Assembly>(AssetID{_json["id"].asString()});
		auto* arrayChange = dynamic_cast<JsonArrayChange*>(change);
		if(assembly)
		{
			if(std::regex_match(path, entityComponentChange))
			{
				Json::ArrayIndex entity = std::stoi(Json::getPathComponent(path, 1));
				auto* arm = Runtime::getModule<AssemblyReloadManager>();
				if(arrayChange)
				{
					bool insert = arrayChange->inserting() != undo;
					if(insert)
						arm->addEntityComponent(assembly, entity, jsonToComponent(arrayChange->value()));
					else
						arm->removeEntityComponent(assembly, entity, am->getAsset<ComponentAsset>(AssetID(arrayChange->value()["id"].asString()))->componentID);
				}
				else
				{
					size_t componentIndex = 0;
					for(auto& component : _json["entities"][entity]["components"])
					{
						arm->updateEntityComponent(assembly, entity, jsonToComponent(_json["entities"][entity]["components"][Json::ArrayIndex(componentIndex++)]));
					};
				}
			}
			if(std::regex_match(path, entityParentChange))
			{
				Json::ArrayIndex entity = std::stoi(Json::getPathComponent(path, 1));
				Runtime::getModule<AssemblyReloadManager>()->updateEntityParent(assembly, entity, _json["entities"][entity]["parent"].asUInt());
			}
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

	auto compID = am->getAsset<ComponentAsset>(AssetID(component["id"].asString()))->componentID;
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
	std::vector<std::pair<AssetID, AssetType>> assets;
	assets.emplace_back(AssetID(_json["id"].asString()), AssetType::assembly);
	if(_json["linked"].asBool())
	{
		for(auto& mesh : _json["linkedMeshes"])
			assets.emplace_back(AssetID(mesh["id"].asString()), AssetType::mesh);
	}
	return assets;
}

Asset* EditorAssemblyAsset::buildAssembly() const
{
	auto* assembly = new Assembly();
	assembly->name = name();
	assembly->id = _json["id"].asString();
	assembly->rootIndex = _json["rootEntity"].asUInt();

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
		assembly->components.push_back(def->asset->id.copy());

	return assembly;
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
	mesh->id = id.copy();

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

void EditorAssemblyAsset::updateEntityComponent(size_t entity, size_t component) const
{
	auto* am = Runtime::getModule<AssetManager>();
	auto* assembly = am->getAsset<Assembly>(AssetID{_json["id"].asString()});
	if(assembly)
	{

		auto* arm = Runtime::getModule<AssemblyReloadManager>();
		arm->updateEntityComponent(assembly, entity, jsonToComponent(_json["entities"][Json::ArrayIndex(entity)]["components"][Json::ArrayIndex(component)]));
	};
}
