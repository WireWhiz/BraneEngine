//
// Created by eli on 2/2/2022.
//

#include "assembly.h"
#include "assetManager.h"
#include "types/scriptAsset.h"
#include <glm/glm.hpp>
#include <ecs/nativeTypes/transform.h>
#include <ecs/nativeTypes/meshRenderer.h>

void WorldEntity::serialize(OSerializedData& message)
{
	message << (uint32_t)components.size();
	for (int i = 0; i < components.size(); ++i)
	{
		message << components[i].def()->id;
		components[i].def()->serializeComponent(message, components[i].data());
	}
}

void WorldEntity::deserialize(ISerializedData& message, AssetManager& am)
{
	uint32_t size;
	message.readSafeArraySize(size);
	components.reserve(size);
	for (uint32_t i = 0; i < size; ++i)
	{
		AssetID componentID;
		message >> componentID;
		ComponentAsset* def = am.getAsset<ComponentAsset>(componentID);
		if(def == nullptr)
			throw std::runtime_error("unknown asset id: " + componentID.string());
		VirtualComponent component(def);
		def->deserializeComponent(message, component.data());
		components.push_back(std::move(component));
	}
}

void WorldEntity::writeToFile(MarkedSerializedData& sData, size_t index)
{
	sData.enterScope("components");
	for (int i = 0; i < components.size(); ++i)
	{
		sData.startIndex();

		sData.writeAttribute("id", components[i].def()->id.string());
		OSerializedData oData;
		components[i].def()->serializeComponent(oData, components[i].data());
		sData.writeAttribute("value", oData.data);

		sData.pushIndex();
	}
	sData.exitScope();
}
void WorldEntity::readFromFile(MarkedSerializedData& sData, size_t index, AssetManager& am)
{

	sData.enterScope("components");
	uint32_t size = sData.scopeSize();
	components.reserve(size);
	for (uint32_t i = 0; i < size; ++i)
	{
		sData.enterScope(i);

		std::string compID;
		sData.readAttribute("id", compID);
		ComponentAsset* def = am.getAsset<ComponentAsset>(AssetID(compID));
		if(def == nullptr)
			throw std::runtime_error("unknown asset id: " + compID);

		VirtualComponent component(def);
		ISerializedData iData;
		sData.readAttribute("value", iData.data);
		def->deserializeComponent(iData, component.data());
		components.push_back(std::move(component));

		sData.exitScope();
	}
	sData.exitScope();
}

std::vector<const ComponentAsset*> WorldEntity::componentDefs()
{
	std::vector<const ComponentAsset*> defs;
	defs.reserve(components.size());
	for(auto& c : components)
	{
		defs.push_back(c.def());
	}
	return defs;
}



void Assembly::toFile(MarkedSerializedData& sData)
{
	Asset::toFile(sData);
	sData.writeAttribute("scripts", scripts);
	sData.writeAttribute("meshes", meshes);
	sData.writeAttribute("textures", textures);
	sData.enterScope("entities");
	for (uint32_t i = 0; i < entities.size(); ++i)
	{
		sData.startIndex();
		entities[i].writeToFile(sData, i);
		sData.pushIndex();
	}
	sData.exitScope();
}

void Assembly::fromFile(MarkedSerializedData& sData, AssetManager& am)
{
	Asset::fromFile(sData, am);
	sData.readAttribute("scripts", scripts);
	sData.readAttribute("meshes", meshes);
	sData.readAttribute("textures", textures);

	sData.enterScope("entities");

	uint32_t count = sData.scopeSize();
	entities.resize(count);
	for (uint32_t i = 0; i < entities.size(); ++i)
	{
		sData.enterScope(i);
		entities[i].readFromFile(sData, i, am);
		sData.exitScope();
	}
	sData.exitScope();
}

void Assembly::serialize(OSerializedData& message)
{
	Asset::serialize(message);
	message << scripts << meshes << textures;
	message << (uint32_t)entities.size();
	for (uint32_t i = 0; i < entities.size(); ++i)
	{
		entities[i].serialize(message);
	}
}

void Assembly::deserialize(ISerializedData& message, AssetManager& am)
{
	Asset::deserialize(message, am);
	message >> scripts >> meshes >> textures;
	uint32_t size;
	message.readSafeArraySize(size);
	entities.resize(size);
	for (uint32_t i = 0; i < entities.size(); ++i)
	{
		entities[i].deserialize(message, am);
	}
}

Json::Value Assembly::toJson(AssetManager& am) const
{
	Json::Value json;
	json["id"] = id.string();
	json["name"] = name;
	json["type"] = "assembly";
	json["dependencies"];
	for(auto& script : scripts)
	{
		Json::Value scriptJson;
		scriptJson["id"] = script.string();
		ScriptAsset* asset = am.getAsset<ScriptAsset>(script);
		if(asset)
			scriptJson["name"] = asset->name;
		else
			scriptJson["name"] = "Asset not found";
		json["dependencies"]["meshes"].append(scriptJson);
	}
	for(auto& mesh : meshes)
	{
		Json::Value meshJson;
		meshJson["id"] = mesh.string();
		MeshAsset* asset = am.getAsset<MeshAsset>(mesh);
		if(asset)
			meshJson["name"] = asset->name;
		else
			meshJson["name"] = "Asset not found";
		json["dependencies"]["meshes"].append(meshJson);
	}
	for(auto& texture : textures)
	{
		assert(false && "texture assets not yet implemented");
		/*Json::Value textureJson;
		textureJson["id"] = texture.string();
		TextureAsset* asset = am.getAsset<TextureAsset>(texture);
		if(asset)
			textureJson["name"] = asset->name;
		else
			textureJson["name"] = "Asset not found";
		json["dependencies"]["textures"].append(textureJson);*/
	}
	json["entities"];
	for(auto& entity : entities)
	{
		Json::Value ent;
		ent["components"];
		for(auto& component : entity.components)
		{
			ent["components"].append(component.def()->toJson(component.data()));
		}
		json["entities"].append(ent);
	}
	return json;
}

Assembly* Assembly::fromJson(Json::Value& json, AssetManager& am)
{
	Assembly* assembly = new Assembly;
	assembly->name = json["name"].asString();
	assembly->id = AssetID(json["id"].asString());

	assembly->scripts.reserve(json["dependencies"]["scripts"].size());
	for(auto& script : json["dependencies"]["scripts"])
	{
		//Todo add verification that dependencies actually exist
		assembly->scripts.emplace_back(script["id"].asString());
	}
	assembly->meshes.reserve(json["dependencies"]["meshes"].size());
	for(auto& mesh : json["dependencies"]["meshes"])
	{
		assembly->meshes.emplace_back(mesh["id"].asString());
	}
	assembly->textures.reserve(json["dependencies"]["textures"].size());
	for(auto& texture : json["dependencies"]["textures"])
	{
		assembly->textures.emplace_back(texture["id"].asString());
	}
	assembly->entities.reserve(json["entities"].size());
	for(auto& entity : json["entities"])
	{
		WorldEntity newEntity;
		newEntity.components.reserve(entity["components"].size());
		for(auto& component : entity["components"])
		{
			const ComponentAsset* componentDef = am.getAsset<ComponentAsset>(component["id"].asString());
			if(!componentDef || componentDef->type != AssetType::component)
				throw std::runtime_error("Asset " + component["id"].asString() + " not found");

			VirtualComponent newComponent(componentDef);
			componentDef->fromJson(component, newComponent.data());

			newEntity.components.push_back(newComponent);
		}
		assembly->entities.push_back(newEntity);
	}

	return assembly;
}

Assembly::Assembly()
{
	type.set(AssetType::Type::assembly);
}

void Assembly::inject(EntityManager& em, EntityID rootID)
{
	std::vector<EntityID> entityMap(entities.size());

	for (int i = 0; i < entities.size(); ++i)
	{
		WorldEntity& entity = entities[i];
		entityMap[i] = em.createEntity(entity.componentDefs());
		if(!em.entityHasComponent(entityMap[i], LocalTransformComponent::def()))
		{
			LocalTransformComponent ltc{};
			ltc.parent = rootID;
			ltc.value = glm::mat4(1);
			entityMap.push_back(rootID);
			em.addComponent(entityMap[i], LocalTransformComponent::def());
			em.setEntityComponent(entityMap[i], ltc.toVirtual());
		}
	}

	for (int i = 0; i < entities.size(); ++i)
	{
		WorldEntity& entity = entities[i];
		EntityID id = entityMap[i];
		for(auto component : entity.components)
		{
			if(component.def() == LocalTransformComponent::def())
			{
				EntityID parent = component.readVar<EntityID>(1);
				parent = entityMap[parent];
				component.setVar(1, parent);
				glm::mat4 transform = component.readVar<glm::mat4>(0);
				//component.setVar(0, glm::translate(glm::mat4(1), {0.3f, 0, 0}));
			}
			em.setEntityComponent(id, component);
		}
	}
}




