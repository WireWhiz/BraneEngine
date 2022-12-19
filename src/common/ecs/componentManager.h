//
// Created by eli on 7/17/2022.
//

#ifndef BRANEENGINE_COMPONENTMANAGER_H
#define BRANEENGINE_COMPONENTMANAGER_H

#include "component.h"
#include "structDefinition.h"
#include "robin_hood.h"

class ComponentManager {
public:
    staticIndexVector<std::unique_ptr<ComponentDescription>> _components;
    robin_hood::unordered_set<ComponentID> _externalComponents;
    robin_hood::unordered_map<std::string, ComponentID> _nameToComponent;
public:
    ~ComponentManager();
    ComponentID registerComponent(ComponentDescription* componentDescription);
    ComponentID registerComponent(const BraneScript::StructDef* typeDef);
    const ComponentDescription* getComponentDef(ComponentID id);
    const ComponentDescription* getComponentDef(const std::string& name);
    void eraseComponent(ComponentID id);
};


#endif //BRANEENGINE_COMPONENTMANAGER_H
