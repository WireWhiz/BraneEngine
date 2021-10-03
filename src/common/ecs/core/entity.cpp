#include "Entity.h"

VirtualArchetype* EntityManager::makeArchetype(std::vector<ComponentDefinition*>& cdefs)
{
	assert(cdefs.size() > 0);
	size_t numComps = cdefs.size();

	// Sort our VirtualStructDefinitions so that we always have a consistant order
	
	std::sort(cdefs.begin(), cdefs.end(), [](const ComponentDefinition* a, const ComponentDefinition* b){
		return a->id() < b->id();
	});
	
	// We want to keep archetypes of the same size in groups
	// This means we keep each size in a different vector

	// Make sure we have enough vectors
	if (numComps >= _archetypes.size())
		_archetypes.resize(numComps);

	size_t newIndex = _archetypes[numComps - 1].size();
	_archetypes[numComps - 1].push_back(std::make_unique<VirtualArchetype>(cdefs));

	VirtualArchetype* newArch = _archetypes[numComps - 1][newIndex].get();
	
	bool isRootArch = false;
	// find edges to other archetypes lower then this one
	if (numComps > 1)
	{
		ComponentID connectingComponent;
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
			ComponentID connectingComponent;
			if (newArch->isChildOf(_archetypes[numComps][i].get(), connectingComponent))
			{
				VirtualArchetype* otherArch = _archetypes[numComps][i].get();
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

void EntityManager::getArchetypeRoots(const std::vector<ComponentID>& components, std::vector<VirtualArchetype*>& roots)
{
	roots.clear();
	std::unordered_set<VirtualArchetype*> found;
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
	for (ComponentID component : components)
	{
		for(VirtualArchetype* archetype : _rootArchetypes[component])
			if (archetype->hasComponents(components) && !found.count(archetype))
				roots.push_back(archetype);
	}
}

void EntityManager::updateArchetypeRoots(VirtualArchetype* archtype)
{
	for (auto& c : archtype->components)
	{
		const ComponentID& component = c.first;
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

void EntityManager::updateForEachRoots(VirtualArchetype* oldArchetype, VirtualArchetype* newArchetype)
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
#ifdef DEBUG
	assert(_components.find(component) != _components.end());
#endif
	_components.erase(component);
}

VirtualArchetype* EntityManager::getArcheytpe(const std::vector<ComponentID>& components)
{
	size_t numComps = components.size();
	assert(numComps > 0);
	if (numComps > _archetypes.size())
		return nullptr;
	auto& archetypes = _archetypes[numComps - 1];

	for (size_t a = 0; a < archetypes.size(); a++)
	{
		assert(archetypes[a] -> components.size() == components.size());
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

EntityID EntityManager::createEntity(const std::vector<ComponentID>& components)
{
	VirtualArchetype* arch = getArcheytpe(components);
	if (arch == nullptr)
	{
		std::vector<ComponentDefinition*> compdefs(components.size());
		for (size_t i = 0; i < components.size(); i++)
		{
			compdefs[i] = _components[components[i]].get();
		}
		arch = makeArchetype(compdefs);
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
	VirtualArchetype* archetype = getEntityArchetype(entity);
	archetype->swapRemove(_entities[entity].index);
}

void EntityManager::forEach(EnityForEachID id, const std::function<void(byte* [])>& f)
{
	assert(id >= 0);
	assert(id < _forEachData.size());
	std::unordered_set<VirtualArchetype*> executed;
	for (VirtualArchetype* archetype : getForEachArchetypes(id))
	{
		forEachRecursive(archetype, _forEachData[id].components, f, executed, true);
	}
}

size_t EntityManager::forEachCount(EnityForEachID id)
{
	size_t count = 0;
	for (VirtualArchetype* archetype : getForEachArchetypes(id))
	{
		count += archetype->size();
	}
	return count;
}

EnityForEachID EntityManager::getForEachID(const std::vector<ComponentID>& components)
{
	//std::sort(components.begin(), components.end());
	EnityForEachID id = 0;
	for (ForEachData& cfe : _forEachData)
	{
		if (cfe.components == components)
			return id;
		++id;
	}
	//If we reatch here there is no prexisting cache
	_forEachData.push_back(ForEachData(components));
	return id;
}

void EntityManager::forEachRecursive(VirtualArchetype* archetype, const std::vector<ComponentID>& components, const std::function<void(byte* [])>& f, std::unordered_set<VirtualArchetype*>& executed, bool searching)
{
	if (executed.find(archetype) != executed.end())
		return;
	
	if (searching) // If we are not the child of a runnable archetype, see if we are
	{
		if (archetype->components.size() >= components.size())
			if (archetype->hasComponents(components))
			{
				archetype->forEach(components, f);
				searching = false;
			}
				
	}
	else // if we are just run it
		archetype->forEach(components, f);
	
	
	archetype->forAddEdge([this, &components, &f, &executed, &searching](std::shared_ptr<ArchetypeEdge> edge) {
		forEachRecursive(edge->archetype, components, f, executed, searching);
	});
	executed.insert(archetype);
}

std::vector<VirtualArchetype*>& EntityManager::getForEachArchetypes(EnityForEachID id)
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


VirtualArchetype* EntityManager::getEntityArchetype(EntityID entity) const
{
	return _entities[entity].archetype;
}

bool EntityManager::hasArchetype(EntityID entity)
{
	return _entities[entity].archetype != nullptr;
}

VirtualComponentPtr EntityManager::getEntityComponent(EntityID entity, ComponentID component) const
{
	(getEntityArchetype(entity)->hasComponent(component));
	return getEntityArchetype(entity)->getComponentVector(component)->getComponent(_entities[entity].index);
}

void EntityManager::addComponent(EntityID entity, ComponentID component)
{
	
	VirtualArchetype* destArchetype = nullptr;
	size_t destArchIndex = 0;
	if (hasArchetype(entity))
	{
		VirtualArchetype* currentArchetype = getEntityArchetype(entity);
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
			std::vector<ComponentDefinition*> compDefs = currentArchetype->getComponentDefs();
			compDefs.push_back(&*_components[component]);
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
			std::vector<ComponentDefinition*> compDefs;
			compDefs.push_back(&*_components[component]);
			destArchetype = makeArchetype(compDefs);
		}
		
	}
	
	size_t oldIndex = _entities[entity].index;
	
	// If this isn't an empty entity we need to copy and remove it
	if (hasArchetype(entity))
	{
		VirtualArchetype* arch = getEntityArchetype(entity);
		_entities[entity].index = destArchetype->copyEntity(arch, _entities[entity].index);
		arch->swapRemove(oldIndex);
	}
	else
	{
		_entities[entity].index = destArchetype->createEntity();
	}
	_entities[entity].archetype = destArchetype;
}

void EntityManager::removeComponent(EntityID entity, ComponentID component)
{
	VirtualArchetype* destArchetype = nullptr;
	size_t destArchIndex = 0;
	assert(hasArchetype(entity)); // Can't remove anything from an entity without any components
	VirtualArchetype* currentArchetype = getEntityArchetype(entity);
	assert(currentArchetype->hasComponent(component)); // can't remove a component that isn't there
	// See if there's already an archetype we know about:
	std::shared_ptr<ArchetypeEdge> ae = currentArchetype->getRemoveEdge(component);
	if (ae != nullptr)
	{
		assert(_entities[entity].archetype->components.size() >= 2); // Must be an archtype level beneath this one
		destArchetype = ae->archetype;
	}
	else
	{
		// Otherwise create one
		std::vector<ComponentDefinition*> compDefs = currentArchetype->getComponentDefs();
		//Remove the component definition for the component that we want to remove
		for (size_t i = 0; i < compDefs.size(); i++)
		{
			if (compDefs[i]->id() == component)
			{
				size_t lastIndex = compDefs.size() - 1;
				compDefs[i] = compDefs[lastIndex];
				compDefs.resize(lastIndex);
			}
		}
		if (compDefs.size() > 0)
		{
			destArchIndex = _archetypes[_entities[entity].archetype->components.size() - 2].size();
			destArchetype = makeArchetype(compDefs);
		}
	}

	size_t oldIndex = _entities[entity].index;
	if (destArchetype != nullptr)
		_entities[entity].index = destArchetype->copyEntity(currentArchetype, _entities[entity].index);
	else
		_entities[entity].index = 0;
	currentArchetype->swapRemove(oldIndex);
	_entities[entity].archetype = destArchetype;
}

void EntityManager::addSystemBlock(const std::string& identifier, const std::string& after, const std::string& before)
{
	_systems.addBlock(identifier, after, before);
}

void EntityManager::removeSystemBlock(const std::string& identifier)
{
	_systems.removeBlock(identifier);
}

SystemBlock* EntityManager::getSystemBlock(const std::string& identifier)
{
	return _systems.find(identifier);
}

void EntityManager::runSystems(VirtualSystemGlobals* constants) const
{
	SystemBlock* currentSystem = _systems[0];
	for (size_t i = 0; i < _systems.size(); i++)
	{
		_systems[i]->runSystems(*this, constants);
	}
}
