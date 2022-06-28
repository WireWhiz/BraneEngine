#pragma once
#include "component.h"
#include "archetype.h"
#include "utility/sharedRecursiveMutex.h"
#include "chunk.h"

#include <utility/staticIndexVector.h>
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <memory>
#include <functional>
#include <unordered_set>
#include <runtime/runtime.h>
#include <runtime/module.h>

#ifndef EntityID
typedef uint32_t EntityID;
#endif

class EntityIDComponent : public NativeComponent<EntityIDComponent>
{
	REGISTER_MEMBERS_1("EntityIDComponent", id)
public:
	EntityID id;
};

struct EntityIndex
{
	Archetype* archetype;
	size_t index;
	bool alive;
};

class EntityManager : public Module
{
#ifdef TEST_BUILD
public:
#endif
	uint16_t _componentIDCount = 0;
	staticIndexVector<EntityIndex> _entities;

	ComponentManager _components;
	ArchetypeManager _archetypes;
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
	bool entityHasComponent(EntityID entity, ComponentID component) const;
	VirtualComponent getEntityComponent(EntityID entity, ComponentID component) const;
	void setEntityComponent(EntityID entity, const VirtualComponent& component);
	void setEntityComponent(EntityID entity, const VirtualComponentView& component);
	void addComponent(EntityID entity, ComponentID component);
	void removeComponent(EntityID entity, ComponentID component);
	ComponentManager& components();

	//for each stuff
	void forEach(const std::vector<ComponentID>& components, const std::function <void(byte* [])>& f);
	void constForEach(const std::vector<ComponentID>& components, const std::function <void(const byte* [])>& f);
	std::shared_ptr<JobHandle> forEachParallel(const std::vector<ComponentID>& components, const std::function <void(byte* [])>& f, size_t entitiesPerThread);
	std::shared_ptr<JobHandle> constForEachParallel(const std::vector<ComponentID>& components, const std::function <void(const byte* [])>& f, size_t entitiesPerThread);

	static const char* name();
	void stop() override;
};