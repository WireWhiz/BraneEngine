#pragma once
#include "Component.h"
#include "Archetype.h"
#include "systemList.h"

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
typedef uint64_t EnityForEachID;

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
	std::vector<EntityIndex> _entities;
	std::queue<EntityID> _unusedEntities;
	std::unordered_map<const ComponentAsset*, std::vector<Archetype*>> _rootArchetypes;

	struct ForEachData
	{
		bool cached;
		ComponentSet components;
		std::vector<Archetype*> archetypeRoots;
		ForEachData(ComponentSet components)
		{
			this->components = std::move(components);
			cached = false;
		}
	};
	std::vector<ForEachData> _forEachData;

	// Index 1: number of components, Index 2: archetype
	std::vector<std::vector<std::unique_ptr<Archetype>>> _archetypes;

	Archetype* makeArchetype(const ComponentSet& cdefs);
	void getArchetypeRoots(const ComponentSet& components, std::vector<Archetype*>& roots) const;
	void updateForeachCache(const ComponentSet& components);
	void forEachRecursive(Archetype* archetype, const ComponentSet& components, const std::function <void(byte* [])>& f, std::unordered_set<Archetype*>& executed, bool searching);
	std::vector<Archetype*>& getForEachArchetypes(EnityForEachID id);
	SystemList _systems;
public:
	EntityManager() = default;
	EntityManager(const EntityManager&) = delete;
	Archetype* getArcheytpe(const ComponentSet& components);
	EntityID createEntity(); 
	EntityID createEntity(const ComponentSet& components);
	void destroyEntity(EntityID entity);
	Archetype* getEntityArchetype(EntityID entity) const;
	bool hasArchetype(EntityID entity) const;  
	size_t archetypeCount(size_t archetypeSize);
	VirtualComponentPtr getEntityComponent(EntityID entity, const ComponentAsset* component) const;
	void addComponent(EntityID entity, const ComponentAsset* component);
	void removeComponent(EntityID entity, const ComponentAsset* component);

	//system stuff
	void forEach(EnityForEachID id, const std::function <void(byte* [])>& f);
	size_t forEachCount(EnityForEachID id);
	EnityForEachID getForEachID(const ComponentSet& components);
	bool addSystem(std::unique_ptr<VirtualSystem>& system);
	void removeSystem(SystemID id);
	bool addBeforeConstraint(SystemID id, SystemID before);
	bool addAfterConstraint(SystemID id, SystemID after);
	VirtualSystem* getSystem(SystemID id);
	void runSystems();
};