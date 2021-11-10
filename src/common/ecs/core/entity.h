#pragma once
#include "component.h"
#include "archetype.h"
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

class EntityIDComponent : public NativeComponent<EntityIDComponent>
{
	REGESTER_MEMBERS_1(id)
public:
	EntityID id;
};

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

	ArchetypeManager _archetypes;
	
	mutable std::mutex _runLock;
	std::queue<std::function<void()>> _runQueue;
	SystemList _systems;
public:
	EntityManager();
	EntityManager(const EntityManager&) = delete;
	~EntityManager();
	Archetype* getArchetype(const ComponentSet& components);
	EntityID createEntity(); 
	EntityID createEntity(ComponentSet components);
	void createEntities(const ComponentSet& components, size_t count);
	void destroyEntity(EntityID entity);
	Archetype* getEntityArchetype(EntityID entity) const;
	bool hasArchetype(EntityID entity) const;  
	VirtualComponent getEntityComponent(EntityID entity, const ComponentAsset* component) const;
	void setEntityComponent(EntityID entity, const VirtualComponent& component);
	void setEntityComponent(EntityID entity, const VirtualComponentPtr& component);
	void addComponent(EntityID entity, const ComponentAsset* component);
	void removeComponent(EntityID entity, const ComponentAsset* component);

	//for each stuff
	EnityForEachID getForEachID(const ComponentSet& components);
	size_t forEachCount(EnityForEachID id);
	void forEach(EnityForEachID id, const std::function <void(byte* [])>& f);
	void constForEach(EnityForEachID id, const std::function <void(const byte* [])>& f);
	std::shared_ptr<JobHandle> forEachParellel(EnityForEachID id, const std::function <void(byte* [])>& f, size_t entitiesPerThread);
	std::shared_ptr<JobHandle> constForEachParellel(EnityForEachID id, const std::function <void(const byte* [])>& f, size_t entitiesPerThread);
	//system stuff
	void run(const std::function<void()>& task);
	bool addSystem(std::unique_ptr<VirtualSystem>&& system);
	void removeSystem(SystemID id);
	bool addBeforeConstraint(SystemID id, SystemID before);
	bool addAfterConstraint(SystemID id, SystemID after);
	VirtualSystem* getSystem(SystemID id);
	void runSystems();
};

class NativeForEach
{
	std::vector<size_t> _componentOrder;
	EnityForEachID _feid;
public:
	NativeForEach() = default;
	NativeForEach(std::vector<const ComponentAsset*>& components, EntityManager* em);
	size_t getComponentIndex(size_t index) const;
	EnityForEachID id() const;
};