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
	_archetypes[numComps - 1].push_back(std::make_unique<Archetype>(cdefs));

	Archetype* newArch = _archetypes[numComps - 1][newIndex].get();
	
	bool isRootArch = false;
	// find edges to other archetypes lower then this one
	if (numComps > 1)
	{
		const ComponentDefinition* connectingComponent;
		for (size_t i = 0; i < _archetypes[numComps - 2].size(); i++)
		{
			if (_archetypes[numComps - 2][i]->isChildOf(newArch, connectingComponent))
			{
				_archetypes[numComps - 2][i]->addAddEdge(connectingComponent, newArch);
				newArch->addRemoveEdge(connectingComponent, _archetypes[numComps - 2][i].get());
			}
		}
	}
	// find edges to other archetypes higher then this one
	if (_archetypes.size() > numComps)
	{
		for (size_t i = 0; i < _archetypes[numComps].size(); i++)
		{
			const ComponentDefinition* connectingComponent;
			if (newArch->isChildOf(_archetypes[numComps][i].get(), connectingComponent))
			{
				Archetype* otherArch = _archetypes[numComps][i].get();
				newArch->addAddEdge(connectingComponent, otherArch);
				otherArch->addRemoveEdge(connectingComponent, newArch);

				updateArchetypeRoots(otherArch);
				updateForEachRoots(otherArch, newArch);
			}
		}
	}
	updateArchetypeRoots(newArch);
	updateForEachRoots(nullptr, newArch);
	return newArch;
}

void EntityManager::getArchetypeRoots(const ComponentSet& components, std::vector<Archetype*>& roots) const
{
	roots.clear();
	std::unordered_set<Archetype*> found;
	// Serch through any archetypes of the exact size that we want to see if there's one that fits
	if (_archetypes.size() >= components.size())
	{
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
	// Search through all the archetypes that have the poetental to have just turned into what we want
	for (const ComponentDefinition* compDef : components)
		for(Archetype * archetype : _rootArchetypes.find(compDef)->second)
			if (!found.count(archetype) && archetype->hasComponents(components))
				roots.push_back(archetype);
	
	
}

void EntityManager::updateArchetypeRoots(Archetype* archtype)
{
	for (auto component : archtype->componentDefs())
	{
		if (archtype->isRootForComponent(component))
		{
			bool isRoot = false;
			for (size_t i = 0; i < _rootArchetypes[component].size(); i++)
			{
				if (_rootArchetypes[component][i] == archtype)
				{
					isRoot = true;
					break;
				}
			}
			if (!isRoot)
				_rootArchetypes[component].push_back(archtype);
		}
		else // remove from root
		{
			for (size_t i = 0; i < _rootArchetypes[component].size(); i++)
			{
				if (_rootArchetypes[component][i] == archtype)
				{
					// swap with last
					_rootArchetypes[component][i] = _rootArchetypes[component][_rootArchetypes[component].size() - 1];
					// remove last
					_rootArchetypes[component].resize(_rootArchetypes[component].size() - 1);
				}
			}
		}
	}
}

void EntityManager::updateForEachRoots(Archetype* oldArchetype, Archetype* newArchetype)
{
	for (ForEachData& data : _forEachData)
	{
		if (oldArchetype != nullptr)
		{
			for (size_t i = 0; i < data.archetypeRoots.size(); i++)
			{
				if (oldArchetype == data.archetypeRoots[i])
				{
					if (newArchetype == nullptr)
					{
						data.archetypeRoots[i] = data.archetypeRoots[data.archetypeRoots.size() - 1];
						data.archetypeRoots.resize(data.archetypeRoots.size() - 1);
					}
					else if (newArchetype->hasComponents(data.components))
					{
						data.archetypeRoots[i] = newArchetype;
					}
				}
			}
		}
		else if(newArchetype->hasComponents(data.components))
		{
			data.archetypeRoots.push_back(newArchetype);
		}
	}
}

void EntityManager::regesterComponent(const ComponentDefinition& newComponent)
{
	assert(_components.find(newComponent.id()) == _components.end());
	// AParEntly std::unordered_map uses std::vector so whenever I add stuff to it, IT REALOCATES and RESIZES the memory Meaning all the pointers that I want to set to the data it contains just BREAK! but no longer... I WILL USE MORE POINTERS, THAT WILL DEFEAT THIS ANTI POINTER TIRANY!!!! this memory does not need to be contigougs like components so no Index maddness here, no maddnes at all... - WireWhiz, 2:40am 
	_components.insert({ newComponent.id(), std::make_unique<ComponentDefinition>(newComponent)});
}

void EntityManager::deregesterComponent(ComponentID component)
{
	assert(_components.find(component) != _components.end());
	_components.erase(component);
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
		assert(archetypes[a] -> componentDefs().size() == components.size());
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
	//std::unordered_set<Archetype*> executed;
	for (size_t size = _forEachData[id].components.size() - 1; size < _archetypes.size(); size++)
	{
		for (size_t i = 0; i < _archetypes[size].size(); i++)
		{
			if (_archetypes[size][i]->hasComponents(_forEachData[id].components))
			{
				_archetypes[size][i]->forEach(_forEachData[id].components, f);
			}
		}
		
	}
	//for (Archetype* archetype : getForEachArchetypes(id))
	//{
	//	forEachRecursive(archetype, _forEachData[id].components, f, executed, true);
	//}
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
		if (archetype->componentDefs().size() >= components.size())
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

VirtualComponentPtr EntityManager::getEntityComponent(EntityID entity, ComponentID componentID) const
{
	assert(getEntityArchetype(entity)->hasComponent(componentDef(componentID)));
	return getEntityArchetype(entity)->getComponent(_entities[entity].index, componentDef(componentID));
}

void EntityManager::addComponent(EntityID entity, ComponentID componentID)
{
	assert(_components.find(componentID) != _components.end());
	const ComponentDefinition* component = componentDef(componentID);
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
			ComponentSet compDefs = currentArchetype->componentDefs();
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

void EntityManager::removeComponent(EntityID entity, ComponentID componentID)
{
	Archetype* destArchetype = nullptr;
	size_t destArchIndex = 0;
	assert(hasArchetype(entity)); // Can't remove anything from an entity without any components
	assert(_components.find(componentID) != _components.end());

	const ComponentDefinition* component = componentDef(componentID);
	Archetype* currentArchetype = getEntityArchetype(entity);

	assert(currentArchetype->hasComponent(component)); // can't remove a component that isn't there
	// See if there's already an archetype we know about:
	std::shared_ptr<ArchetypeEdge> ae = currentArchetype->getRemoveEdge(component);
	if (ae != nullptr)
	{
		assert(_entities[entity].archetype->componentDefs().size() >= 2); // Must be an archtype level beneath this one
		destArchetype = ae->archetype;
	}
	else
	{
		// Otherwise create one
		ComponentSet compDefs = currentArchetype->componentDefs();
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

const ComponentDefinition* EntityManager::componentDef(ComponentID componentID) const
{
	assert(_components.find(componentID) != _components.end());
	return _components.find(componentID)->second.get();
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
