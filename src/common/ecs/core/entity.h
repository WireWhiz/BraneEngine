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

	struct ForEachData
	{
		bool cached = false;
		std::vector<ComponentID> components;
		std::vector<VirtualArchetype*> archetypeRoots;
		ForEachData(std::vector<ComponentID> components)
		{
			this->components = std::move(components);
		}
	};
	std::vector<ForEachData> _forEachData;

	// Index 1: number of components, Index 2: archetype
	std::vector<std::vector<std::unique_ptr<VirtualArchetype>>> _archetypes;

	VirtualArchetype* makeArchetype(std::vector<ComponentDefinition*>& cdefs);
	void getArchetypeRoots(const std::vector<ComponentID>& components, std::vector<VirtualArchetype*>& roots);
	void updateArchetypeRoots(VirtualArchetype* archtype);
	void updateForEachRoots(VirtualArchetype* oldArchetype, VirtualArchetype* newArchetype);
	void forEachRecursive(VirtualArchetype* archetype, const std::vector<ComponentID>& components, const std::function <void(byte* [])>& f, std::unordered_set<VirtualArchetype*>& executed, bool searching);
	std::vector<VirtualArchetype*>& getForEachArchetypes(EnityForEachID id);
	SystemList _systems;
public:
	EntityManager() = default;
	EntityManager(const EntityManager&) = delete;
	void regesterComponent(const ComponentDefinition& newComponent);
	void deregesterComponent(ComponentID component);
	VirtualArchetype* getArcheytpe(const std::vector<ComponentID>& components);
	EntityID createEntity(); 
	EntityID createEntity(const std::vector<ComponentID>& components);
	void destroyEntity(EntityID entity);
	VirtualArchetype* getEntityArchetype(EntityID entity) const;
	bool hasArchetype(EntityID entity);  
	VirtualComponentPtr getEntityComponent(EntityID entity, ComponentID component) const;
	void addComponent(EntityID entity, ComponentID component);
	void removeComponent(EntityID entity, ComponentID component);

	//system stuff
	void forEach(EnityForEachID id, const std::function <void(byte* [])>& f);
	size_t forEachCount(EnityForEachID id);
	EnityForEachID getForEachID(const std::vector<ComponentID>& components);
	void addSystem(std::unique_ptr<VirtualSystem>& system);
	void removeSystemBlock(SystemID id);
	VirtualSystem* getSystem(SystemID id);
	void runSystems();
};