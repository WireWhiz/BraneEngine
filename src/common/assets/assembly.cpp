//
// Created by eli on 2/2/2022.
//

#include "assembly.h"
#include "assetManager.h"
#include "ecs/core/componentManager.h"
#include "systems/transforms.h"
#include "graphics/graphics.h"
#include <glm/glm.hpp>
#include <ecs/nativeTypes/meshRenderer.h>

void Assembly::EntityAsset::serialize(OutputSerializer message, Assembly& assembly)
{
    uint32_t size = static_cast<uint32_t>(components.size());
	message << size;
	for (size_t i = 0; i < size; ++i)
	{
		uint32_t componentIDIndex = 0;
		for(auto& id : assembly.components)
		{
			if(id == components[i].description()->asset->id)
				break;
			++componentIDIndex;
		}
		if(componentIDIndex >= assembly.components.size())
		{
			Runtime::error("Component: " + components[i].description()->asset->id.string() + " not found in asset components");
			throw std::runtime_error("Component assetID not listed in asset components");
		}

        uint32_t ssize = components[i].description()->serializationSize();
		message << componentIDIndex << ssize;
		components[i].description()->serialize(message, components[i].data());
	}
}

void Assembly::EntityAsset::deserialize(InputSerializer message, Assembly& assembly, ComponentManager& cm, AssetManager& am)
{
	uint32_t size;
	message.readSafeArraySize(size);
	components.reserve(size);
	for (uint32_t i = 0; i < size; ++i)
	{
		uint32_t componentIDIndex, componentSize;
		message >> componentIDIndex >> componentSize;
        if(componentIDIndex >= assembly.components.size())
            throw std::runtime_error("Component ID index invalid");
		const ComponentDescription* description = cm.getComponentDef(am.getAsset<ComponentAsset>(assembly.components[componentIDIndex])->componentID);
		if(componentSize != description->serializationSize())
        {
            Runtime::error("Component size " + std::to_string(componentSize) + " does not match size of component "
            + description->name + " which is " + std::to_string(description->serializationSize()) + ".\n attempting to continue deserialization");
            message.setPos(message.getPos() + componentSize);
            continue;
        }
        VirtualComponent component(description);
		description->deserialize(message, component.data());
		components.push_back(std::move(component));
	}
}

bool Assembly::EntityAsset::hasComponent(const ComponentDescription* def) const
{
	for (const auto& c : components)
		if (c.description() == def)
			return true;
	return false;
}

VirtualComponent* Assembly::EntityAsset::getComponent(const ComponentDescription* def)
{
	assert(hasComponent(def));
	for (auto& c : components)
		if (c.description() == def)
			return &c;
    return nullptr;
}

std::vector<ComponentID> Assembly::EntityAsset::runtimeComponentIDs()
{
	std::vector<ComponentID> componentIDs;
	componentIDs.reserve(components.size());
	for(auto& component : components)
	{
		componentIDs.push_back(component.description()->id);
	}
	return componentIDs;
}

void Assembly::serialize(OutputSerializer message)
{
	Asset::serialize(message);
	message << components << scripts << meshes << textures;
	message << (uint32_t)entities.size();
	for (uint32_t i = 0; i < entities.size(); ++i)
	{
		entities[i].serialize(message, *this);
	}
}

void Assembly::deserialize(InputSerializer message)
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

#ifdef CLIENT
void Assembly::initialize(EntityManager& em, graphics::VulkanRuntime& vkr, AssetManager& am)
{
	//Temporarily use an example material
	auto* mat = new graphics::Material();
	mat->setVertex(vkr.loadShader(0));
	mat->setFragment(vkr.loadShader(1));
	//mat->addTextureDescriptor(vkr.loadTexture(0));
	mat->addBinding(0,sizeof(glm::vec3));
	mat->addBinding(1, sizeof(glm::vec3));
	mat->addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, 0);
	mat->addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, 0);
	vkr.addMaterial(mat);
	em.components().registerComponent(mat->component());

	for(auto& entity : entities)
	{
		for(auto& component : entity.components)
		{
			if(component.description() == Transform::def())
			{
				component.setVar(1, true);
			}
			else if(component.description() == TRS::def())
			{
				component.setVar(3, true);
			}
			else  if(component.description() == MeshRendererComponent::def())
			{
				MeshRendererComponent* mr = MeshRendererComponent::fromVirtual(component.data());
				auto* meshAsset = am.getAsset<MeshAsset>(AssetID(meshes[mr->mesh]));
				if(meshAsset->pipelineID == -1){
					vkr.addMesh(meshAsset);

				}
				mr->mesh = meshAsset->pipelineID;
				entity.components.emplace_back(mat->component());

				//TODO: process materials here as well
			}
		}
	}
}
#endif

#ifdef SERVER
void Assembly::initialize(EntityManager& em, AssetManager& am)
{
    for(auto& entity : entities)
    {
        for(auto& component : entity.components)
        {
            if(component.description() == Transform::def())
            {
                component.setVar(1, true);
            }
            else if(component.description() == TRS::def())
            {
                component.setVar(3, true);
            }
        }
    }
}
#endif

void Assembly::inject(EntityManager& em, EntityID rootID)
{
	std::vector<EntityID> entityMap(entities.size());
	for (size_t i = 0; i < entities.size(); ++i)
	{
		EntityAsset& entity = entities[i];
		entityMap[i] = em.createEntity(entity.runtimeComponentIDs());
		if(!em.hasComponent(entityMap[i], LocalTransform::def()->id))
		{
			Transforms::setParent(entityMap[i], rootID, em);
		}
	}

	for (size_t i = 0; i < entities.size(); ++i)
	{
		EntityAsset& entity = entities[i];
		EntityID id = entityMap[i];
		for(auto& component : entity.components)
			em.setComponent(id, component);
	}

	//Set transforms
	for(size_t i = 0; i < entities.size(); ++i)
	{
		EntityAsset& entity = entities[i];
		EntityID id = entityMap[i];
		if(entity.hasComponent(LocalTransform::def()))
		{
			EntityID parent = entityMap[entity.getComponent(LocalTransform::def())->readVar<EntityID>(1)];
			Transforms::setParent(id, parent, em);
		}
	}
}




