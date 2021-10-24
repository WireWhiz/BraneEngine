#include "Entity.h"

Archetype* EntityManager::makeArchetype(const ComponentSet& cdefs)
{
	assert(cdefs.size() > 0);
	size_t numComps = cdefs.size();
	
	// We want to keep archetypes of the same size in groups
	// This means we keep each size in a different vector

	// Make sure we have enough vectors
	if (numComps > _archetypes.size())
		_archetypes.resize(numComps);

	size_t newIndex = _archetypes[numComps - 1].size();
	_archetypes[numComps - 1].push_back(std::make_unique<Archetype>(cdefs, _chunkAllocator));

	Archetype* newArch = _archetypes[numComps - 1][newIndex].get();


	
	{//Scope for bool array
		bool* isRootArch = (bool*)STACK_ALLOCATE(sizeof(bool) * cdefs.size());
		for (size_t i = 0; i < cdefs.size(); i++)
		{
			isRootArch[i] = true;
		}

		// find edges to other archetypes lower then this one
		if (numComps > 1)
		{
			const ComponentAsset* connectingComponent = nullptr;
			for (size_t i = 0; i < _archetypes[numComps - 2].size(); i++)
			{
				if (_archetypes[numComps - 2][i]->isChildOf(newArch, connectingComponent))
				{
					assert(connectingComponent != nullptr);
					Archetype* otherArch = _archetypes[numComps - 2][i].get();
					otherArch->addAddEdge(connectingComponent, newArch);
					newArch->addRemoveEdge(connectingComponent, otherArch);

					for (size_t i = 0; i < cdefs.size(); i++)
					{
						if(isRootArch[i] && connectingComponent != cdefs[i])
							isRootArch[i] = false;
					}
				}
			}
		}

		// find edges to other archetypes higher then this one
		if (_archetypes.size() > numComps)
		{
			for (size_t i = 0; i < _archetypes[numComps].size(); i++)
			{
				const ComponentAsset* connectingComponent = nullptr;
				if (newArch->isChildOf(_archetypes[numComps][i].get(), connectingComponent))
				{
					assert(connectingComponent != nullptr);
					Archetype* otherArch = _archetypes[numComps][i].get();
					newArch->addAddEdge(connectingComponent, otherArch);
					otherArch->addRemoveEdge(connectingComponent, newArch);

					for (const ComponentAsset* component : cdefs.components())
					{
						if (!_rootArchetypes.count(component))
							continue;
						std::vector<Archetype*>& componentRoots = _rootArchetypes[component];
						for (size_t i = 0; i < componentRoots.size(); i++)
						{
							if (componentRoots[i] == otherArch)
							{
								componentRoots[i] == *(componentRoots.end() - 1);
								componentRoots.resize(componentRoots.size() - 1);

							}
						}
					}
				}
			}
		}

		for (size_t i = 0; i < cdefs.size(); i++)
		{
			if (isRootArch[i])
				_rootArchetypes[cdefs[i]].push_back(newArch);
		}
	}
	updateForeachCache(cdefs);

	return newArch;
}

void EntityManager::getArchetypeRoots(const ComponentSet& components, std::vector<Archetype*>& roots) const
{
	roots.clear();
	std::unordered_set<Archetype*> found;
	// Serch through any archetypes of the exact size that we want to see if there's one that fits
	if (_archetypes.size() >= components.size())
	{
		// Searches through all archetypes with the same number of components as the for each
		for (auto& archetype : _archetypes[components.size() - 1])
		{
			if (archetype->hasComponents(components))
			{
				roots.push_back(archetype.get());
				found.insert(archetype.get());
				break;
			}
		}
	}
	// Search through all the archetypes that have components we want, but won't be found through an add edge chain
	for (const ComponentAsset* compDef : components)
	{
		if (_rootArchetypes.count(compDef))
		{
			for (Archetype* archetype : _rootArchetypes.find(compDef)->second)
			{
				if (!found.count(archetype) && archetype->hasComponents(components))
				{
					roots.push_back(archetype);
					found.insert(archetype);
				}
			}
		}
	}
		
	
	
}

void EntityManager::updateForeachCache(const ComponentSet& components)
{
	for (ForEachData& data : _forEachData)
	{
		if (components.contains(data.components))
			data.cached = false;
	}
}

EntityManager::EntityManager()
{
	_chunkAllocator = std::make_unique<ChunkPool>();
}

Archetype* EntityManager::getArcheytpe(const ComponentSet& components)
{
	size_t numComps = components.size();
	assert(numComps > 0);
	if (numComps > _archetypes.size())
		return nullptr;
	auto& archetypes = _archetypes[numComps - 1];

	for (size_t a = 0; a < archetypes.size(); a++)
	{
		assert(archetypes[a] -> components().size() == components.size());
		if(archetypes[a]->hasComponents(components))
			return archetypes[a].get();
	}
	return nullptr;
}

EntityID EntityManager::createEntity()
{
	EntityID id;
	if (_unusedEntities.size() > 0)
	{
		id = _unusedEntities.front();
		_unusedEntities.pop();
	}
	else
	{
		id = _entities.size();
		_entities.push_back(EntityIndex());
	}
	
	EntityIndex& eIndex = _entities[id];
	eIndex.archetype = nullptr;
	eIndex.index = 0;
	eIndex.alive = true;

	return id;
}

EntityID EntityManager::createEntity(const ComponentSet& components)
{
	Archetype* arch = getArcheytpe(components);
	if (arch == nullptr)
	{
		arch = makeArchetype(components);
	}
	EntityID ent = createEntity();
	_entities[ent].archetype = arch;
	_entities[ent].index = arch->createEntity();
	return ent;
}

void EntityManager::destroyEntity(EntityID entity)
{
	assert(0 <= entity && entity < _entities.size());
	_entities[entity].alive = false;
	_unusedEntities.push(entity);
	Archetype* archetype = getEntityArchetype(entity);
	archetype->remove(_entities[entity].index);
}

void EntityManager::forEach(EnityForEachID id, const std::function<void(byte* [])>& f)
{
	assert(id >= 0);
	assert(id < _forEachData.size());
	//for (size_t size = _forEachData[id].components.size() - 1; size < _archetypes.size(); size++)
	//{
	//	for (size_t i = 0; i < _archetypes[size].size(); i++)
	//	{
	//		if (_archetypes[size][i]->hasComponents(_forEachData[id].components))
	//		{
	//			_archetypes[size][i]->forEach(_forEachData[id].components, f);
	//		}
	//	}
	//	
	//}
	std::unordered_set<Archetype*> executed;
	for (Archetype* archetype : getForEachArchetypes(id))
	{
		forEachRecursive(archetype, _forEachData[id].components, f, executed, true);
	}
}

size_t EntityManager::forEachCount(EnityForEachID id)
{
	size_t count = 0;
	for (Archetype* archetype : getForEachArchetypes(id))
	{
		count += archetype->size();
	}
	return count;
}

EnityForEachID EntityManager::getForEachID(const ComponentSet& components)
{
	//std::sort(components.begin(), components.end());
	EnityForEachID id = 0;
	for (ForEachData& d : _forEachData)
	{
		if (d.components.size() == components.size() && d.components.contains(components))
			return id;
		++id;
	}
	//If we reatch here there is no prexisting cache
	_forEachData.push_back(ForEachData(components));
	return id;
}

void EntityManager::forEachRecursive(Archetype* archetype, const ComponentSet& components, const std::function<void(byte* [])>& f, std::unordered_set<Archetype*>& executed, bool searching)
{
	if (executed.find(archetype) != executed.end())
		return;
	
	if (searching) // If we are not the child of a runnable archetype, see if we are
	{
		if (archetype->components().size() >= components.size())
			if (archetype->hasComponents(components))
			{
				archetype->forEach(components, f);
				searching = false;
			}
				
	}
	else // if we are just run it
		archetype->forEach(components, f);
	
	executed.insert(archetype);
	
	archetype->forAddEdge([this, &components, &f, &executed, &searching](std::shared_ptr<ArchetypeEdge> edge) {
		forEachRecursive(edge->archetype, components, f, executed, searching);
	});
	
}

std::vector<Archetype*>& EntityManager::getForEachArchetypes(EnityForEachID id)
{
	assert(_forEachData.size() > id);
	// If the list is more then
	if (!_forEachData[id].cached)
	{
		//create new cache
		 getArchetypeRoots(_forEachData[id].components, _forEachData[id].archetypeRoots);

		_forEachData[id].cached = true;
	}

	return _forEachData[id].archetypeRoots;
}


Archetype* EntityManager::getEntityArchetype(EntityID entity) const
{
	assert(entity < _entities.size());
	return _entities[entity].archetype;
}

bool EntityManager::hasArchetype(EntityID entity) const
{
	return _entities[entity].archetype != nullptr;
}


VirtualComponentPtr EntityManager::getEntityComponent(EntityID entity, const ComponentAsset* component) const
{
	assert(component != nullptr);
	assert(getEntityArchetype(entity)->hasComponent(component));
	return getEntityArchetype(entity)->getComponent(_entities[entity].index, component);
}

void EntityManager::addComponent(EntityID entity, const ComponentAsset* component)
{
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
			destArchetype = makeArchetype(compDefs);
			
		}
	}
	else
	{
		// the entity has no components, so we need to find or create a new one.
		if(_archetypes.size() > 0)
			for (size_t i = 0; i < _archetypes[0].size(); i++)
			{
				if (_archetypes[0][i]->hasComponent(component))
				{
					destArchetype = _archetypes[0][i].get();
				}
			}
		if (destArchetype == nullptr)
		{
			ComponentSet compDefs;
			compDefs.add(component);
			destArchetype = makeArchetype(compDefs);
		}
		
	}
	
	size_t oldIndex = _entities[entity].index;
	
	// If this isn't an empty entity we need to copy and remove it
	if (hasArchetype(entity))
	{
		Archetype* arch = getEntityArchetype(entity);
		_entities[entity].index = destArchetype->copyEntity(arch, _entities[entity].index);
		arch->remove(oldIndex);
	}
	else
	{
		_entities[entity].index = destArchetype->createEntity();
	}
	_entities[entity].archetype = destArchetype;
}

void EntityManager::removeComponent(EntityID entity, const ComponentAsset* component)
{
	Archetype* destArchetype = nullptr;
	size_t destArchIndex = 0;
	assert(hasArchetype(entity)); // Can't remove anything from an entity without any components
	
		
	Archetype* currentArchetype = getEntityArchetype(entity);

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
			destArchetype = makeArchetype(compDefs);
		}
	}

	size_t oldIndex = _entities[entity].index;
	if (destArchetype != nullptr)
		_entities[entity].index = destArchetype->copyEntity(currentArchetype, _entities[entity].index);
	else
		_entities[entity].index = 0;
	currentArchetype->remove(oldIndex);
	_entities[entity].archetype = destArchetype;
}

bool EntityManager::addSystem(std::unique_ptr<VirtualSystem>& system)
{
	return _systems.addSystem(system);
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
	_systems.runSystems(this);
}
