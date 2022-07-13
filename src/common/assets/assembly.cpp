//
// Created by eli on 2/2/2022.
//

#include "assembly.h"
#include "assetManager.h"
#include "systems/transforms.h"
#include <glm/glm.hpp>
#include <ecs/nativeTypes/meshRenderer.h>

void Assembly::WorldEntity::serialize(OSerializedData& message, Assembly& assembly)
{
	message << (uint32_t)components.size();
	for (size_t i = 0; i < components.size(); ++i)
	{
		uint32_t componentIDIndex = 0;
		for(auto& id : assembly.components)
		{
			if(id == components[i].description()->asset->id)
				break;
			++componentIDIndex;
		}
		if(componentIDIndex == assembly.components.size())
		{
			Runtime::error("Component: " + components[i].description()->asset->id.string() + " not found in asset components");
			throw std::runtime_error("Component assetID not listed in asset components");
		}

		message << componentIDIndex;
		components[i].description()->serialize(message, components[i].data());
	}
}

void Assembly::WorldEntity::deserialize(ISerializedData& message, Assembly& assembly, ComponentManager& cm, AssetManager& am)
{
	uint32_t size;
	message.readSafeArraySize(size);
	components.reserve(size);
	for (uint32_t i = 0; i < size; ++i)
	{
		uint32_t componentIDIndex;
		message >> componentIDIndex;
		const ComponentDescription* description = cm.getComponent(am.getAsset<ComponentAsset>(assembly.components[componentIDIndex])->componentID);
		VirtualComponent component(description);
		description->deserialize(message, component.data());
		components.push_back(std::move(component));
	}
}

void Assembly::WorldEntity::writeToFile(MarkedSerializedData& sData, Assembly& assembly)
{
	sData.enterScope("components");
	for (int i = 0; i < components.size(); ++i)
	{
		sData.startIndex();

		uint32_t componentIDIndex = 0;
		for(auto& id : assembly.components)
		{
			if(id == components[i].description()->asset->id)
				break;
			++componentIDIndex;
		}
		if(componentIDIndex == assembly.components.size())
		{
			Runtime::error("Component: " + components[i].description()->asset->id.string() + " not found in asset components");
			throw std::runtime_error("Component assetID not listed in asset components");
		}
		sData.writeAttribute("assetID", componentIDIndex);

		OSerializedData oData;
		components[i].description()->serialize(oData, components[i].data());
		sData.writeAttribute("value", oData.data);

		sData.pushIndex();
	}
	sData.exitScope();
}
void Assembly::WorldEntity::readFromFile(MarkedSerializedData& sData, Assembly& assembly, ComponentManager& cm, AssetManager& am)
{
	sData.enterScope("components");
	uint32_t size = sData.scopeSize();
	components.reserve(size);
	for (uint32_t i = 0; i < size; ++i)
	{
		sData.enterScope(i);
		uint32_t componentIDIndex;
		sData.readAttribute("assetID", componentIDIndex);
		const ComponentDescription* description = cm.getComponent(am.getAsset<ComponentAsset>(assembly.components[componentIDIndex])->componentID);

		VirtualComponent component(description);
		ISerializedData iData;
		sData.readAttribute("value", iData.data);
		description->deserialize(iData, component.data());
		components.push_back(std::move(component));

		sData.exitScope();
	}
	sData.exitScope();
}

std::vector<ComponentID> Assembly::WorldEntity::runtimeComponentIDs()
{
	std::vector<ComponentID> componentIDs;
	componentIDs.reserve(components.size());
	for(auto& component : components)
	{
		componentIDs.push_back(component.description()->id);
	}
	return componentIDs;
}

void Assembly::toFile(MarkedSerializedData& sData)
{
	Asset::toFile(sData);
	sData.writeAttribute("scripts", scripts);
	sData.writeAttribute("meshes", meshes);
	sData.writeAttribute("textures", textures);
	sData.writeAttribute("components", components);
	sData.enterScope("entities");
	for (uint32_t i = 0; i < entities.size(); ++i)
	{
		sData.startIndex();
		entities[i].writeToFile(sData, *this);
		sData.pushIndex();
	}
	sData.exitScope();
}

void Assembly::fromFile(MarkedSerializedData& sData)
{
	ComponentManager& cm = Runtime::getModule<EntityManager>()->components();
	AssetManager& am = *Runtime::getModule<AssetManager>();

	Asset::fromFile(sData);
	sData.readAttribute("scripts", scripts);
	sData.readAttribute("meshes", meshes);
	sData.readAttribute("textures", textures);
	sData.readAttribute("components", components);
	sData.enterScope("entities");

	uint32_t count = sData.scopeSize();
	entities.resize(count);
	for (uint32_t i = 0; i < entities.size(); ++i)
	{
		sData.enterScope(i);
		entities[i].readFromFile(sData, *this, cm, am);
		sData.exitScope();
	}
	sData.exitScope();
}

void Assembly::serialize(OSerializedData& message)
{
	Asset::serialize(message);
	message << components << scripts << meshes << textures;
	message << (uint32_t)entities.size();
	for (uint32_t i = 0; i < entities.size(); ++i)
	{
		entities[i].serialize(message, *this);
	}
}

void Assembly::deserialize(ISerializedData& message)
{
	ComponentManager& cm = Runtime::getModule<EntityManager>()->components();
	AssetManager& am = *Runtime::getModule<AssetManager>();

	Asset::deserialize(message);
	message >> components >> scripts >> meshes >> textures;
	uint32_t size;
	message.readSafeArraySize(size);
	entities.resize(size);
	for (uint32_t i = 0; i < entities.size(); ++i)
	{
		entities[i].deserialize(message, *this, cm, am);
	}
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
		entityMap[i] = em.createEntity(entity.runtimeComponentIDs());
		if(!em.entityHasComponent(entityMap[i], LocalTransform::def()->id))
		{
			LocalTransform ltc{};
			ltc.parent = rootID;
			ltc.value = glm::mat4(1);
			entityMap.push_back(rootID);
			em.addComponent(entityMap[i], LocalTransform::def()->id);
			em.setEntityComponent(entityMap[i], ltc.toVirtual());
		}
	}

	for (int i = 0; i < entities.size(); ++i)
	{
		WorldEntity& entity = entities[i];
		EntityID id = entityMap[i];
		for(auto component : entity.components)
		{
			if(component.description() == LocalTransform::def())
			{
				EntityID parent = component.readVar<EntityID>(1);
				parent = entityMap[parent];
				component.setVar(1, parent);
				glm::mat4 transform = component.readVar<glm::mat4>(0);
			}
			em.setEntityComponent(id, component);
		}
	}
}




