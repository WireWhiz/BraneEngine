//
// Created by eli on 7/17/2022.
//

#include "componentManager.h"
#include "assets/types/componentAsset.h"

ComponentID ComponentManager::createComponent(ComponentAsset* component)
{
	ComponentID id = createComponent(component->members(), component->name);
	component->componentID = id;
	return id;
}

ComponentID ComponentManager::createComponent(const std::vector<VirtualType::Type>& component, const std::string& name)
{
	ComponentID id = static_cast<ComponentID>(_components.push(std::make_unique<ComponentDescription>(component)));
	_components[id]->id = id;
	_components[id]->name = name;
	return id;
}

const ComponentDescription* ComponentManager::getComponentDef(ComponentID id)
{
	return _components[id].get();
}

void ComponentManager::eraseComponent(ComponentID id)
{
	if(_externalComponents.count(id))
		_components[id].release();
	_components.remove(id);
}

ComponentID ComponentManager::registerComponent(ComponentDescription* componentDescription)
{
	ComponentID id = static_cast<ComponentID>(_components.push(std::unique_ptr<ComponentDescription>(componentDescription)));
	_components[id]->id = id;
	componentDescription->id = id;
	_externalComponents.insert(id);
	return id;
}

ComponentManager::~ComponentManager()
{
	for(auto c : _externalComponents)
		_components[c].release();
}