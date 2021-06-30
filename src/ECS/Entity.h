#pragma once
#include <cstdint>
#include <vector>
#include <Component.h>
#include <VirtualSystem.h>
#include <unordered_map>
#include <Archetype.h>
#include <queue>
#include <algorithm>


typedef uint64_t EntityID;

struct EntityIndex
{
	// Can't use a pointer here, because std::vector likes to be all smart and reallocate memory breaking pointers mercilessly, so I have to deal with this madness to store the archetype that contains the entity
	size_t componentCount;
	size_t archetypeIndex;
	size_t index;
	bool alive;
};

class EntityManager
{
	std::vector<EntityIndex> _entities;
	std::queue<EntityID> _unusedEntities;
	std::unordered_map<ComponentID, std::unique_ptr<ComponentDefinition>> _components;
	// Index 1: number of components, Index 2: archetype
	std::vector<std::vector<VirtualArchetype>> _archetypes;

	VirtualArchetype* makeArchetype(std::vector<ComponentDefinition*>& cdefs);

public:
	void regesterComponent(const ComponentDefinition& newComponent);
	void deregesterComponent(ComponentID component);

	EntityID createEntity();
	void destroyEntity(EntityID entity);
	VirtualArchetype* getEntityArchetype(EntityID entity) const;
	bool hasArchetype(EntityID entity);  
	VirtualComponentPtr getComponent(EntityID entity, ComponentID component) const;
	void addComponent(EntityID entity, ComponentID component);
	void removeComponent(EntityID entity, ComponentID component);
};