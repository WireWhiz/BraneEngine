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
	std::vector<EntityIndex> _entities;
	std::queue<EntityID> _unusedEntities;
	std::unordered_map<ComponentID, std::unique_ptr<ComponentDefinition>> _components;
	std::unordered_map<const ComponentDefinition*, std::vector<Archetype*>> _rootArchetypes;

	struct ForEachData
	{
		bool cached = false;
		ComponentSet components;
		std::vector<Archetype*> archetypeRoots;
		ForEachData(ComponentSet components)
		{
			this->components = std::move(components);
		}
	};
	std::vector<ForEachData> _forEachData;

	// Index 1: number of components, Index 2: archetype
	std::vector<std::vector<std::unique_ptr<Archetype>>> _archetypes;

	Archetype* makeArchetype(const ComponentSet& cdefs);
	void getArchetypeRoots(const ComponentSet& components, std::vector<Archetype*>& roots) const;
	void updateArchetypeRoots(Archetype* archtype);
	void updateForEachRoots(Archetype* oldArchetype, Archetype* newArchetype);
	void forEachRecursive(Archetype* archetype, const ComponentSet& components, const std::function <void(byte* [])>& f, std::unordered_set<Archetype*>& executed, bool searching);
	std::vector<Archetype*>& getForEachArchetypes(EnityForEachID id);
	SystemList _systems;
public:
	EntityManager() = default;
	EntityManager(const EntityManager&) = delete;
	void regesterComponent(const ComponentDefinition& newComponent);
	void deregesterComponent(ComponentID component);
	Archetype* getArcheytpe(const ComponentSet& components);
	EntityID createEntity(); 
	EntityID createEntity(const ComponentSet& components);
	void destroyEntity(EntityID entity);
	Archetype* getEntityArchetype(EntityID entity) const;
	bool hasArchetype(EntityID entity);  
	VirtualComponentPtr getEntityComponent(EntityID entity, ComponentID componentID) const;
	void addComponent(EntityID entity, ComponentID componentID);
	void removeComponent(EntityID entity, ComponentID componentID);
	const ComponentDefinition* componentDef(ComponentID componentID);

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