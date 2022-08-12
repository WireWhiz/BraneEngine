//
// Created by eli on 2/2/2022.
//

#ifndef BRANEENGINE_ASSEMBLY_H
#define BRANEENGINE_ASSEMBLY_H

#include "asset.h"
#include <json/json.h>
#include "ecs/component.h"

class EntityManager;
class ComponentManager;
class AssetManager;
namespace graphics{
	class VulkanRuntime;
}

class Assembly : public Asset
{
public:
	struct EntityAsset
	{
		std::vector<VirtualComponent> components;
		std::vector<ComponentID> runtimeComponentIDs();
		void serialize(OutputSerializer& message, Assembly& assembly);
		void deserialize(InputSerializer& message, Assembly& assembly, ComponentManager& cm, AssetManager& am);
		bool hasComponent(const ComponentDescription* def) const;
		VirtualComponent* getComponent(const ComponentDescription* def);
	};

	Assembly();
	std::vector<AssetID> components;
	std::vector<AssetID> scripts; // Any systems in dependencies will be automatically loaded
	std::vector<AssetID> meshes; // We need to store these in a list, so we can tell witch asset entities are referring to
    std::vector<AssetID> materials;
	std::vector<EntityAsset> entities;
	void serialize(OutputSerializer& message) override;
	void deserialize(InputSerializer& message) override;

    std::vector<AssetDependency> dependencies() const override;
    void onDependenciesLoaded() override;
	std::vector<EntityID> inject(EntityManager& em, EntityID rootID);
};


#endif //BRANEENGINE_ASSEMBLY_H
