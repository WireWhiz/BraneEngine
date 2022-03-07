//
// Created by eli on 2/2/2022.
//

#define  GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "assembly.h"
#include "assetManager.h"
#include <glm/glm.hpp>

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
		for(auto component : entity.components)
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


