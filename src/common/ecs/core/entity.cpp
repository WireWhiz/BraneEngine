#include "entity.h"

EntityID& EntityID::operator=(uint32_t value) {
    id = value;
    return *this;
}

EntityID::operator uint32_t() const{
    return id;
}

EntityManager::EntityManager() : _components(), _archetypes(_components)
{
	Runtime::timeline().addTask("systems", [this](){
        _systems.runSystems(*this);
    }, "main");
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
	id.id = EntityID{static_cast<uint32_t>(_entities.push(EntityIndex())), ++_globalEntityVersion};

	EntityIndex& eIndex = _entities[id.id];
	eIndex.archetype = arch;
	eIndex.index = arch->createEntity();
	eIndex.version = _globalEntityVersion;
	arch->setComponent(eIndex.index, id.toVirtual());
	assert(eIndex.index < arch->size());
    for(auto& e : _entities)
    {
        assert(e.index < e.archetype->size());
    }

	return id.id;
}

void EntityManager::createEntities(const ComponentSet& components, size_t count)
{
	ASSERT_MAIN_THREAD();
	Archetype* arch = getArchetype(components);

	for (size_t i = 0; i < count; i++)
	{
		EntityIDComponent id{};
		id.id = EntityID{static_cast<uint32_t>(_entities.push(EntityIndex())), ++_globalEntityVersion};
		_entities[id.id].archetype = arch;
		_entities[id.id].index = arch->createEntity();
        _entities[id.id].version = _globalEntityVersion;
		assert(_entities[id.id].index < arch->size());
	}
}

void EntityManager::destroyEntity(EntityID entity)
{
	ASSERT_MAIN_THREAD();

	assert(entityExists(entity));
	Archetype* archetype = getEntityArchetype(entity);
    size_t index = _entities[entity].index;
	archetype->removeEntity(index);
	_entities.remove(entity);

    //If we just moved an entity's data, update it's index;
    if(index < archetype->size())
    {
        EntityID swappedEntity = *archetype->getComponent(index, EntityIDComponent::def()->id).getVar<EntityID>(0);
        assert(_entities[swappedEntity].archetype == archetype);
        _entities[swappedEntity].index = index;
    }

    for(auto& e : _entities)
    {
        assert(e.index < e.archetype->size());
    }
}

Archetype* EntityManager::getEntityArchetype(EntityID entity) const
{
	assert(entityExists(entity));
	Archetype* o = _entities[entity].archetype;
	return o;
}

bool EntityManager::hasArchetype(EntityID entity) const
{
	assert(entityExists(entity));
	bool o = _entities[entity].archetype != nullptr;
	return o;
}

bool EntityManager::hasComponent(EntityID entity, ComponentID component) const
{
	return getEntityArchetype(entity)->hasComponent(component);
}

VirtualComponentView EntityManager::getComponent(EntityID entity, ComponentID component) const
{
	assert(getEntityArchetype(entity)->hasComponent(component));
	return getEntityArchetype(entity)->getComponent(_entities[entity].index, component);
}

void EntityManager::setComponent(EntityID entity, const VirtualComponent& component)
{
	assert(getEntityArchetype(entity)->hasComponent(component.description()->id));
	getEntityArchetype(entity)->setComponent(_entities[entity].index, component);
}

void EntityManager::setComponent(EntityID entity, const VirtualComponentView& component)
{
	assert(getEntityArchetype(entity)->hasComponent(component.description()->id));
	getEntityArchetype(entity)->setComponent(_entities[entity].index, component);
}

void EntityManager::addComponent(EntityID entity, ComponentID component)
{
	ASSERT_MAIN_THREAD();
    assert(entityExists(entity));
	Archetype* destArchetype = nullptr;
	if (hasArchetype(entity))
	{
		Archetype* currentArchetype = getEntityArchetype(entity);
        assert(!currentArchetype->hasComponent(component)); // can't add a component that we already have
        assert(_entities[entity].index < currentArchetype->size());
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
	size_t newIndex;
	size_t oldIndex = _entities[entity].index;
    EntityID swappedEntity;
	// If this isn't an empty entity we need to copy and remove it
	if (hasArchetype(entity))
	{
		Archetype* arch = getEntityArchetype(entity);
		newIndex = arch->moveEntity(oldIndex, destArchetype);
		//Check if move entity performed a swap.
		if(oldIndex < arch->size())
		{
			swappedEntity = *arch->getComponent(oldIndex, EntityIDComponent::def()->id).getVar<EntityID>(0);
			assert(arch == _entities[swappedEntity].archetype);
            _entities[swappedEntity].index = oldIndex;
		}
	}
	else
		newIndex = destArchetype->createEntity();

	assert(newIndex < destArchetype->size());
	_entities[entity].index = newIndex;
	_entities[entity].archetype = destArchetype;
}

void EntityManager::removeComponent(EntityID entity, ComponentID component)
{
	ASSERT_MAIN_THREAD();
	Archetype* destArchetype = nullptr;
	size_t destArchIndex = 0;
    assert(entityExists(entity));
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
	{
		newIndex = currentArchetype->moveEntity(oldIndex, destArchetype);
        if(oldIndex < currentArchetype->size())
        {
            EntityID swappedEntity = *currentArchetype->getComponent(oldIndex, EntityIDComponent::def()->id).getVar<EntityID>(0);
            assert(currentArchetype == _entities[swappedEntity.id].archetype);
            _entities[swappedEntity].index = oldIndex;
        }
	}


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

SystemManager& EntityManager::systems()
{
	return _systems;
}

ArchetypeManager& EntityManager::archetypes()
{
	return _archetypes;
}

EntitySet EntityManager::getEntities(ComponentFilter filter)
{
	return _archetypes.getEntities(std::move(filter));
}

bool EntityManager::entityExists(EntityID entity) const
{
    return _entities.hasIndex(entity) && entity.version == _entities[entity].version;
}

bool EntityManager::tryGetEntity(size_t index, EntityID& id) const
{
    if(!_entities.hasIndex(index)){
        return false;
    }
    id.id = index;
    id.version = _entities[index].version;
    return true;
}
