//
// Created by eli on 2/2/2022.
//

#include "assembly.h"
#include "assetManager.h"
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

void Assembly::serialize(OSerializedData& message)
{
	Asset::serialize(message);
	message << scripts << meshes << textures;
	message << (uint32_t)data.size();
	for (uint32_t i = 0; i < data.size(); ++i)
	{
		data[i].serialize(message);
	}
}

void Assembly::deserialize(ISerializedData& message, AssetManager& am)
{
	Asset::deserialize(message, am);
	message >> scripts >> meshes >> textures;
	uint32_t size;
	message.readSafeArraySize(size);
	data.resize(size);
	for (uint32_t i = 0; i < data.size(); ++i)
	{
		data[i].deserialize(message, am);
	}
}

Json::Value Assembly::toJson(Assembly* assembly)
{
	Json::Value json;
	json["dependencies"];
	for(auto& script : assembly->scripts)
		json["dependencies"]["scripts"].append(script.string());
	for(auto& mesh : assembly->meshes)
		json["dependencies"]["meshes"].append(mesh.string());
	for(auto& texture : assembly->textures)
		json["dependencies"]["textures"].append(texture.string());
	json["entities"];
	for(auto& entity : assembly->data)
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

Assembly* Assembly::fromJson(Json::Value& json)
{
	Assembly assembly;

	return nullptr;
}

Assembly::Assembly()
{
	type.set(AssetType::Type::assembly);
}

void Assembly::inject(EntityManager& em)
{
	std::vector<EntityID> entityMap(data.size());

	for (int i = 0; i < data.size(); ++i)
	{
		WorldEntity& entity = data[i];
		entityMap[i] = em.createEntity(entity.componentDefs());
	}

	for (int i = 0; i < data.size(); ++i)
	{
		WorldEntity& entity = data[i];
		EntityID id = entityMap[i];
		for(auto component : entity.components)
		{
			if(component.def() == comps::LocalTransformComponent::def())
			{
				EntityID parent = component.readVar<EntityID>(1);
				parent = entityMap[parent];
				component.setVar(1, parent);
				glm::mat4 transform = component.readVar<glm::mat4>(0);
				//component.setVar(0, glm::translate(glm::mat4(1), {0.3f, 0, 0}));
			}
			else if(component.def() == comps::MeshRendererComponent::def())
			{
				//TODO: translate from local material indexes to ones from the vulkan runtime
			}
			else if(component.def() == comps::TransformComponent::def())
			{

				//component.setVar(0, glm::translate(glm::mat4(1), {0, 0, 0}));
			}


			em.setEntityComponent(id, component);
		}
		em.createEntity(entity.componentDefs());
	}

}


