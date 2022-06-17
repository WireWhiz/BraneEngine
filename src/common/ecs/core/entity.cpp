#include "entity.h"

EntityManager::EntityManager() : _components(), _archetypes(_components)
{
	EntityIDComponent::constructDescription();
	_components.registerComponent(EntityIDComponent::def());
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
	components.add(EntityIDComponent::def()->id);
	Archetype* arch = getArchetype(components);
	EntityIDComponent id{};
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
		EntityIDComponent id{};
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

void EntityManager::forEach(const std::vector<ComponentID>& components, const std::function<void(byte* [])>& f)
{
	_archetypes.forEach(components, f);
}

void EntityManager::constForEach(const std::vector<ComponentID>& components, const std::function<void(const byte* [])>& f)
{
	_archetypes.constForEach(components, f);
}

std::shared_ptr<JobHandle> EntityManager::forEachParallel(const std::vector<ComponentID>& components, const std::function<void(byte* [])>& f, size_t entitiesPerThread)
{
	return _archetypes.forEachParallel(components, f, entitiesPerThread);
}

std::shared_ptr<JobHandle> EntityManager::constForEachParallel(const std::vector<ComponentID>& components, const std::function<void(const byte* [])>& f, size_t entitiesPerThread)
{
	return _archetypes.constForEachParallel(components, f, entitiesPerThread);
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

bool EntityManager::entityHasComponent(EntityID entity, ComponentID component) const
{
	return getEntityArchetype(entity)->hasComponent(component);
}

VirtualComponent EntityManager::getEntityComponent(EntityID entity, ComponentID component) const
{
	assert(getEntityArchetype(entity)->hasComponent(component));
	VirtualComponent o = getEntityArchetype(entity)->getComponent(_entities[entity].index, component);
	return o;
}

void EntityManager::setEntityComponent(EntityID entity, const VirtualComponent& component)
{
	assert(getEntityArchetype(entity)->hasComponent(component.description()->id));
	getEntityArchetype(entity)->setComponent(_entities[entity].index, component);
}

void EntityManager::setEntityComponent(EntityID entity, const VirtualComponentView& component)
{
	assert(getEntityArchetype(entity)->hasComponent(component.description()->id));
	getEntityArchetype(entity)->setComponent(_entities[entity].index, component);
}

void EntityManager::addComponent(EntityID entity, ComponentID component)
{
	ASSERT_MAIN_THREAD();
	assert(entity < _entities.size());
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

void EntityManager::removeComponent(EntityID entity, ComponentID component)
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

const char* EntityManager::name()
{
	return "entityManager";
}

void EntityManager::stop()
{
	_archetypes.clear();
}

ComponentManager& EntityManager::components()
{
	return _components;
}
