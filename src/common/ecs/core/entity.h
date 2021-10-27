#pragma once
#include "Component.h"
#include "Archetype.h"
#include "systemList.h"
#include "utility/shared_recursive_mutex.h"
#include "chunk.h"

#include <stdexcept>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <memory>
#include <functional>
#include <unordered_set>

typedef uint64_t EntityID;

struct EntityIndex
{
	Archetype* archetype;
	size_t index;
	bool alive;
};

class EntityManager
{
#ifdef TEST_BUILD
public:
#endif
	mutable shared_recursive_mutex _entityLock;
	std::vector<EntityIndex> _entities;
	std::queue<EntityID> _unusedEntities;

	ArchetypeManager _archetypes;
	
	SystemList _systems;
public:
	EntityManager();
	EntityManager(const EntityManager&) = delete;
	Archetype* getArchetype(const ComponentSet& components);
	EntityID createEntity(); 
	EntityID createEntity(const ComponentSet& components);
	void destroyEntity(EntityID entity);
	Archetype* getEntityArchetype(EntityID entity) const;
	bool hasArchetype(EntityID entity) const;  
	VirtualComponent getEntityComponent(EntityID entity, const ComponentAsset* component) const;
	void setEntityComponent(EntityID entity, const VirtualComponent& component);
	void addComponent(EntityID entity, const ComponentAsset* component);
	void removeComponent(EntityID entity, const ComponentAsset* component);

	//for each stuff
	EnityForEachID getForEachID(const ComponentSet& components);
	size_t forEachCount(EnityForEachID id);
	void forEach(EnityForEachID id, const std::function <void(byte* [])>& f);
	void constForEach(EnityForEachID id, const std::function <void(const byte* [])>& f);
	//system stuff
	bool addSystem(std::unique_ptr<VirtualSystem>& system);
	void removeSystem(SystemID id);
	bool addBeforeConstraint(SystemID id, SystemID before);
	bool addAfterConstraint(SystemID id, SystemID after);
	VirtualSystem* getSystem(SystemID id);
	void runSystems();
};