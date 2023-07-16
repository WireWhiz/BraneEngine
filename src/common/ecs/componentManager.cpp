//
// Created by eli on 7/17/2022.
//

#include "componentManager.h"
#include "assets/types/componentAsset.h"

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


ComponentID ComponentManager::registerComponent(const BraneScript::StructDef* typeDef)
{
    ComponentID index = _components.push(std::make_unique<ComponentDescription>(typeDef));
    _components[index]->id = index;
    _nameToComponent.insert({typeDef->name(), index});
    return index;
}


ComponentID ComponentManager::registerComponent(ComponentDescription* componentDescription)
{
    ComponentID id = static_cast<ComponentID>(_components.push(std::unique_ptr<ComponentDescription>(componentDescription)));
    _components[id]->id = id;
    _externalComponents.insert(id);
    _nameToComponent.insert({componentDescription->name(), id});
    return id;
}

ComponentManager::~ComponentManager()
{
    for(auto c : _externalComponents)
        _components[c].release();
}

const ComponentDescription* ComponentManager::getComponentDef(const std::string& name)
{
    auto itr = _nameToComponent.find(name);
    if(itr == _nameToComponent.end())
        return nullptr;
    return _components[itr->second].get();
}
