#pragma once
#include <cstdint>
#include <vector>
#include "Component.h"
#include "Archetype.h"
#include "VirtualSystem.h"
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <memory>
#include <functional>
#include <unordered_set>


typedef uint64_t EntityID;

struct EntityIndex
{
	VirtualArchetype* archetype;
	size_t index;
	bool alive;
};

class EntityManager
{
	std::vector<EntityIndex> _entities;
	std::queue<EntityID> _unusedEntities;
	std::unordered_map<ComponentID, std::unique_ptr<ComponentDefinition>> _components;
	std::unordered_map<ComponentID, std::vector<VirtualArchetype*>> _rootArchetypes;
	// Index 1: number of components, Index 2: archetype
	std::vector<std::vector<std::unique_ptr<VirtualArchetype>>> _archetypes;

	VirtualArchetype* makeArchetype(std::vector<ComponentDefinition*>& cdefs);
	void updateArchetypeRoots(VirtualArchetype* archtype);
	void forEachRecursive(VirtualArchetype* archetype, const std::vector<ComponentID>& components, const std::function <void(byte* [])>& f, std::unordered_set<VirtualArchetype*>& executed);
public:
	void regesterComponent(const ComponentDefinition& newComponent);
	void deregesterComponent(ComponentID component);
	VirtualArchetype* getArcheytpe(const std::vector<ComponentID>& components);
	EntityID createEntity(); 
	void destroyEntity(EntityID entity);
	void forEach(const std::vector<ComponentID>& components, const std::function <void(byte* [])>& f);
	void runSystem(VirtualSystem* vs, VirtualSystemGlobals* constants);
	VirtualArchetype* getEntityArchetype(EntityID entity) const;
	bool hasArchetype(EntityID entity);  
	VirtualComponentPtr getEntityComponent(EntityID entity, ComponentID component) const;
	void addComponent(EntityID entity, ComponentID component);
	void removeComponent(EntityID entity, ComponentID component);
};