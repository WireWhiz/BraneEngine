#include "entity.h"

EntityManager::EntityManager()
{

}

EntityManager::~EntityManager()
{

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
	id.id = (EntityID)_entities.push(EntityIndex());

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
		id.id = (EntityID)_entities.push(EntityIndex());
		_entities[id.id].archetype = arch;
		_entities[id.id].index = arch->createEntity();
	}
}

void EntityManager::destroyEntity(EntityID entity)
{
	ASSERT_MAIN_THREAD();

	assert(0 <= entity && entity < _entities.size());
	_entities.remove(entity);
	Archetype* archetype = getEntityArchetype(entity);
	archetype->remove(_entities[entity].index);

}

EntityForEachID EntityManager::getForEachID(const ComponentSet& components, const ComponentSet& exclude)
{
	return _archetypes.getForEachID(components, exclude);
}

size_t EntityManager::forEachCount(EntityForEachID id)
{
	ASSERT_MAIN_THREAD();
	return _archetypes.forEachCount(id);
}

void EntityManager::forEach(EntityForEachID id, const std::function<void(byte* [])>& f)
{
	ASSERT_MAIN_THREAD();
	_archetypes.forEach(id, f);
}

void EntityManager::constForEach(EntityForEachID id, const std::function<void(const byte* [])>& f)
{
	ASSERT_MAIN_THREAD();
	_archetypes.constForEach(id, f);
}

std::shared_ptr<JobHandle> EntityManager::forEachParallel(EntityForEachID id, const std::function<void(byte* [])>& f, size_t entitiesPerThread)
{
	ASSERT_MAIN_THREAD();
	return _archetypes.forEachParallel(id, f, entitiesPerThread);
}

std::shared_ptr<JobHandle> EntityManager::constForEachParallel(EntityForEachID id, const std::function<void(const byte* [])>& f, size_t entitiesPerThread)
{
	ASSERT_MAIN_THREAD();
	return _archetypes.constForEachParallel(id, f, entitiesPerThread);
}

Archetype* EntityManager::getEntityArchetype(EntityID entity) const
{
	assert(entity < _entities.size());
	Archetype* o = _entities[entity].archetype;
	return o;
}

bool EntityManager::hasArchetype(EntityID entity) const
{
	assert(entity < _entities.size());
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
	assert(entity < _entities.size());
	assert(component != nullptr);
	Archetype* destArchetype = nullptr;
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
		assert(_entities[entity].archetype->components().size() >= 2); // Must be an archetype level beneath this one
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
NativeForEach::NativeForEach(std::vector<const ComponentAsset* >&& vector, EntityManager* em) : NativeForEach::NativeForEach(vector, em) {}
NativeForEach::NativeForEach(std::vector<const ComponentAsset*>& components, EntityManager* em)
{
	ComponentSet componentSet;
	for (size_t i = 0; i < components.size(); i++)
	{
		componentSet.add(components[i]);
	}

	_componentOrder.resize(components.size());
	for (size_t i = 0; i < components.size(); i++)
	{
		_componentOrder[i] = componentSet.index(components[i]);
	}

	_forEachId = em->getForEachID(componentSet);
}

NativeForEach::NativeForEach(std::vector<const ComponentAsset*>&& components, ComponentSet&& exclude, EntityManager* em) : NativeForEach(components, exclude, em) {}

NativeForEach::NativeForEach(std::vector<const ComponentAsset*>& components, ComponentSet& exclude, EntityManager* em)
{
	ComponentSet componentSet;
	for (size_t i = 0; i < components.size(); i++)
	{
		componentSet.add(components[i]);
	}

	_componentOrder.resize(components.size());
	for (size_t i = 0; i < components.size(); i++)
	{
		_componentOrder[i] = componentSet.index(components[i]);
	}

    _forEachId = em->getForEachID(componentSet, exclude);
}

size_t NativeForEach::getComponentIndex(size_t index) const
{
	assert(index < _componentOrder.size());
	return _componentOrder[index];
}

EntityForEachID NativeForEach::id() const
{
	return _forEachId;
}
