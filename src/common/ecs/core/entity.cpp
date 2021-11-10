#include "entity.h"

EntityManager::EntityManager()
{
	ThreadPool::init();
}

EntityManager::~EntityManager()
{
	ThreadPool::cleanup();
}

Archetype* EntityManager::getArchetype(const ComponentSet& components)
{
	return _archetypes.getArchetype(components);
}

EntityID EntityManager::createEntity()
{
	ComponentSet components;
	return createEntity(components);
}

EntityID EntityManager::createEntity(ComponentSet components)
{
	ASSERT_MAIN_THREAD();
	components.add(EntityIDComponent::def());
	Archetype* arch = getArchetype(components);
	EntityIDComponent id;

	if (_unusedEntities.size() > 0)
	{
		id.id = _unusedEntities.front();
		_unusedEntities.pop();
	}
	else
	{
		id.id = _entities.size();
		_entities.push_back(EntityIndex());
	}

	EntityIndex& eIndex = _entities[id.id];
	eIndex.archetype = arch;
	eIndex.index = arch->createEntity();
	eIndex.alive = true;
	arch->setComponent(eIndex.index, id.toVirtual());
	return id.id;
}

void EntityManager::createEntities(const ComponentSet& components, size_t count)
{
	ASSERT_MAIN_THREAD();
	Archetype* arch = getArchetype(components);

	for (size_t i = 0; i < count; i++)
	{
		EntityIDComponent id;
		if (_unusedEntities.size() > 0)
		{
			id.id = _unusedEntities.front();
			_unusedEntities.pop();
		}
		else
		{
			id.id = _entities.size();
			_entities.push_back(EntityIndex());
		}
		_entities[id.id].archetype = arch;
		_entities[id.id].index = arch->createEntity();
	}
}

void EntityManager::destroyEntity(EntityID entity)
{
	ASSERT_MAIN_THREAD();

	assert(0 <= entity && entity < _entities.size());
	_entities[entity].alive = false;
	_unusedEntities.push(entity);
	Archetype* archetype = getEntityArchetype(entity);
	archetype->remove(_entities[entity].index);

}

EnityForEachID EntityManager::getForEachID(const ComponentSet& components)
{
	return _archetypes.getForEachID(components);
}

size_t EntityManager::forEachCount(EnityForEachID id)
{
	ASSERT_MAIN_THREAD();
	return _archetypes.forEachCount(id);
}

void EntityManager::forEach(EnityForEachID id, const std::function<void(byte* [])>& f)
{
	ASSERT_MAIN_THREAD();
	_archetypes.forEach(id, f);
}

void EntityManager::constForEach(EnityForEachID id, const std::function<void(const byte* [])>& f)
{
	ASSERT_MAIN_THREAD();
	_archetypes.constForEach(id, f);
}

std::shared_ptr<JobHandle> EntityManager::forEachParellel(EnityForEachID id, const std::function<void(byte* [])>& f, size_t entitiesPerThread)
{
	ASSERT_MAIN_THREAD();
	return _archetypes.forEachParellel(id, f, entitiesPerThread);
}

std::shared_ptr<JobHandle> EntityManager::constForEachParellel(EnityForEachID id, const std::function<void(const byte* [])>& f, size_t entitiesPerThread)
{
	ASSERT_MAIN_THREAD();
	return _archetypes.constForEachParellel(id, f, entitiesPerThread);
}

Archetype* EntityManager::getEntityArchetype(EntityID entity) const
{
	assert(entity < _entities.size());
	Archetype* o = _entities[entity].archetype;
	return o;
}

bool EntityManager::hasArchetype(EntityID entity) const
{
	bool o = _entities[entity].archetype != nullptr;
	return o;
}


VirtualComponent EntityManager::getEntityComponent(EntityID entity, const ComponentAsset* component) const
{
	assert(component != nullptr);
	assert(getEntityArchetype(entity)->hasComponent(component));
	VirtualComponent o = getEntityArchetype(entity)->getComponent(_entities[entity].index, component);
	return o;
}

void EntityManager::setEntityComponent(EntityID entity, const VirtualComponent& component)
{
	assert(getEntityArchetype(entity)->hasComponent(component.def()));
	getEntityArchetype(entity)->setComponent(_entities[entity].index, component);
}

void EntityManager::setEntityComponent(EntityID entity, const VirtualComponentPtr& component)
{
	assert(getEntityArchetype(entity)->hasComponent(component.def()));
	getEntityArchetype(entity)->setComponent(_entities[entity].index, component);
}

void EntityManager::addComponent(EntityID entity, const ComponentAsset* component)
{
	ASSERT_MAIN_THREAD();
	assert(component != nullptr);
	Archetype* destArchetype = nullptr;
	size_t destArchIndex = 0;
	if (hasArchetype(entity))
	{
		Archetype* currentArchetype = getEntityArchetype(entity);
		assert(!currentArchetype->hasComponent(component)); // can't add a component that we already have
		// See if there's already an archetype we know about:
		std::shared_ptr<ArchetypeEdge> ae = currentArchetype->getAddEdge(component);
		if (ae != nullptr)
		{
			destArchetype = ae->archetype;
		}
		else
		{
			// Otherwise create one
			ComponentSet compDefs = currentArchetype->components();
			compDefs.add(component);
			destArchetype = _archetypes.makeArchetype(compDefs);
			
		}
	}
	else
	{
		// the entity has no components, so we need to find or create a new one.
		ComponentSet compDefs;
		compDefs.add(component);
		destArchetype = _archetypes.getArchetype(compDefs);
	}
	size_t oldIndex = _entities[entity].index;
	size_t newIndex = 0;
	
	// If this isn't an empty entity we need to copy and remove it
	if (hasArchetype(entity))
	{
		Archetype* arch = getEntityArchetype(entity);
		newIndex = destArchetype->copyEntity(arch, oldIndex);
		arch->remove(oldIndex);
	}
	else
	{
		newIndex = destArchetype->createEntity();
	}
	_entities[entity].index = newIndex;
	_entities[entity].archetype = destArchetype;
}

void EntityManager::removeComponent(EntityID entity, const ComponentAsset* component)
{
	ASSERT_MAIN_THREAD();
	Archetype* destArchetype = nullptr;
	size_t destArchIndex = 0;
	assert(hasArchetype(entity)); // Can't remove anything from an entity without any components
	
		
	Archetype* currentArchetype = getEntityArchetype(entity);
	assert(currentArchetype);

	assert(currentArchetype->hasComponent(component)); // can't remove a component that isn't there
	// See if there's already an archetype we know about:
	std::shared_ptr<ArchetypeEdge> ae = currentArchetype->getRemoveEdge(component);
	if (ae != nullptr)
	{
		assert(_entities[entity].archetype->components().size() >= 2); // Must be an archtype level beneath this one
		destArchetype = ae->archetype;
	}
	else
	{
		// Otherwise create one
		ComponentSet compDefs = currentArchetype->components();
		//Remove the component definition for the component that we want to remove
		compDefs.remove(component);
		if (compDefs.size() > 0)
		{
			destArchetype = _archetypes.makeArchetype(compDefs);
		}
	}
	size_t oldIndex = _entities[entity].index;
	size_t newIndex = 0;

	if (destArchetype != nullptr)
		newIndex = destArchetype->copyEntity(currentArchetype, oldIndex);
	currentArchetype->remove(oldIndex);

	_entities[entity].index = newIndex;
	_entities[entity].archetype = destArchetype;
}

void EntityManager::run(const std::function<void()>& task)
{
	_runLock.lock();
	_runQueue.push(task);
	_runLock.unlock();
}

bool EntityManager::addSystem(std::unique_ptr<VirtualSystem>&& system)
{
	return _systems.addSystem(std::move(system));
}

void EntityManager::removeSystem(SystemID id)
{
	_systems.removeSystem(id);
}

bool EntityManager::addBeforeConstraint(SystemID id, SystemID before)
{
	return _systems.addBeforeConstraint(id, before);
}

bool EntityManager::addAfterConstraint(SystemID id, SystemID after)
{
	return _systems.addAfterConstraint(id, after);
}

VirtualSystem* EntityManager::getSystem(SystemID id)
{
	return _systems.findSystem(id);
}

void EntityManager::runSystems()
{
	while (!_runQueue.empty())
	{
		_runQueue.front()();
		_runQueue.pop();
	}
	_systems.runSystems(this);
}
/*
void EntityIDComponent::getComponentData(std::vector<std::unique_ptr<VirtualType>>& types, AssetID& id)
{
	types.push_back(std::make_unique<VirtualVariable<EntityID>>(offsetof(EntityIDComponent, id)));
}
*/
NativeForEach::NativeForEach(std::vector<const ComponentAsset*>& components, EntityManager* em)
{
	ComponentSet componentSet;
	for (size_t i = 0; i < components.size(); i++)
	{
		componentSet.add(components[i]);
	}
	_componentOrder.resize(components.size());
	componentSet.indicies(componentSet, _componentOrder.data());
	_feid = em->getForEachID(componentSet);
}

size_t NativeForEach::getComponentIndex(size_t index) const
{
	return _componentOrder[index];
}

EnityForEachID NativeForEach::id() const
{
	return _feid;
}
