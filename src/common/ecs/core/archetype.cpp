#include "Archetype.h"

#include "chunk.h"


size_t Archetype::chunkIndex(size_t entity) const
{
	return entity / (Chunk::allocationSize() / _entitySize);
}

Archetype::Archetype(const ComponentSet& components, std::shared_ptr<ChunkPool>& chunkAllocator) : _components(components)
{
	_chunkAllocator = chunkAllocator;
	_entitySize = 0;
	for (size_t i = 0; i < components.size(); i++)
	{
		_entitySize += components[i]->size();
	}

}

Archetype::~Archetype()
{
	while (!_chunks.empty())
	{
		*_chunkAllocator << _chunks[_chunks.size() - 1];
		_chunks.erase(_chunks.end() - 1);
	}
}

bool Archetype::hasComponent(const ComponentAsset* component) const
{
	return _components.contains(component);
}

bool Archetype::hasComponents(const ComponentSet& comps) const
{
	return _components.contains(comps);
}

VirtualComponent Archetype::getComponent(size_t entity, const ComponentAsset* component) const
{
	_mutex.lock_shared();
	assert(_components.contains(component));

	size_t chunk = chunkIndex(entity);
	assert(chunk < _chunks.size());

	size_t index = entity - chunk * _chunks[0]->maxCapacity();
	assert(index < _chunks[chunk]->size());
	VirtualComponent o = _chunks[chunk]->getComponent(component, index);
	_mutex.unlock_shared();
	return o;
}

void Archetype::setComponent(size_t entity, const VirtualComponent& component)
{
	_mutex.lock();
	assert(_components.contains(component.def()));

	size_t chunk = chunkIndex(entity);
	assert(chunk < _chunks.size());

	size_t index = entity - chunk * _chunks[0]->maxCapacity();
	assert(index < _chunks[chunk]->size());
	_chunks[chunk]->setComponent(component, index);
	_mutex.unlock();
}

void Archetype::setComponent(size_t entity, const VirtualComponentPtr& component)
{
	_mutex.lock();
	assert(_components.contains(component.def()));

	size_t chunk = chunkIndex(entity);
	assert(chunk < _chunks.size());

	size_t index = entity - chunk * _chunks[0]->maxCapacity();
	assert(index < _chunks[chunk]->size());
	_chunks[chunk]->setComponent(component, index);
	_mutex.unlock();
}

bool Archetype::isChildOf(const Archetype* parent, const ComponentAsset*& connectingComponent) const
{
	_mutex.lock_shared();
	assert(_components.size() + 1 == parent->_components.size()); //Make sure this is a valid comparason
	byte missCount = 0;
	for (size_t i = 0; i - missCount < _components.size(); i++)
	{
		assert(i - missCount < _components.size());
			
		if (_components[i - missCount] != parent->_components[i])
		{
			connectingComponent = parent->_components.components()[i];
			if (++missCount > 1)
			{
				_mutex.unlock_shared();
				return false;
			}
		}
	}
	if(!connectingComponent)
		connectingComponent = parent->_components[parent->_components.size() - 1];
	_mutex.unlock_shared();
	return true;
}

bool Archetype::isRootForComponent(const ComponentAsset* component) const
{
	_mutex.lock_shared();
	if (_components.size() == 1)
		return true;
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		if (_removeEdges[i]->component == component)
		{
			_mutex.unlock_shared();
			return true;
		}
	}
	_mutex.unlock_shared();
	return false;
}


const ComponentSet& Archetype::components() const
{
	return _components;
}

std::shared_ptr<ArchetypeEdge> Archetype::getAddEdge(const ComponentAsset* component)
{
	_mutex.lock_shared();
	for (size_t i = 0; i < _addEdges.size(); i++)
	{
		if (component == _addEdges[i]->component)
		{
			_mutex.unlock_shared();
			return _addEdges[i];
		}
	}
	_mutex.unlock_shared();
	return nullptr;
}

std::shared_ptr<ArchetypeEdge> Archetype::getRemoveEdge(const ComponentAsset* component)
{
	_mutex.lock_shared();
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		if (component == _removeEdges[i]->component)
		{
			_mutex.unlock_shared();
			return _removeEdges[i];
		}
	}
	_mutex.unlock_shared();
	return nullptr;
}

void Archetype::addAddEdge(const ComponentAsset* component, Archetype* archetype)
{
	_mutex.lock();
	_addEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
	_mutex.unlock();
}

void Archetype::addRemoveEdge(const ComponentAsset* component, Archetype* archetype)
{
	_mutex.lock();
	_removeEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
	_mutex.unlock();
}

void Archetype::forAddEdge(const std::function<void(const std::shared_ptr<ArchetypeEdge>)>& f) const
{
	_mutex.lock_shared();
	for (size_t i = 0; i < _addEdges.size(); i++)
	{
		f(_addEdges[i]);
	}
	_mutex.unlock_shared();
}

void Archetype::forRemoveEdge(std::function<void(const std::shared_ptr<ArchetypeEdge>)>& f) const
{
	_mutex.lock_shared();
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		f(_removeEdges[i]);
	}
	_mutex.unlock_shared();
}

size_t Archetype::size()
{
	_mutex.lock_shared();
	size_t o = _size;
	_mutex.unlock_shared();
	return o;
}

size_t Archetype::createEntity()
{
	_mutex.lock();
	size_t chunk = chunkIndex(_size);
	if (chunk >= _chunks.size())
	{
		_chunks.resize(_chunks.size() + 1);
		*_chunkAllocator >> _chunks[_chunks.size() - 1];
		_chunks[_chunks.size() - 1]->setArchetype(this);
	}

	assert(chunk < _chunks.size());
	_chunks[chunk]->createEntity();

	size_t size = _size++;
	_mutex.unlock();
	return size;
}

size_t Archetype::copyEntity(Archetype* source, size_t index)
{
	_mutex.lock();
	source->_mutex.lock();
	size_t newIndex = createEntity();
	
	size_t srcChunkIndex = source->chunkIndex(index);
	size_t destChunkIndex = chunkIndex(newIndex);

	Chunk* src = source->_chunks[srcChunkIndex].get();
	Chunk* dest = _chunks[destChunkIndex].get();  

	//Index within chunk
	size_t srcEntityIndex = index - srcChunkIndex * src->maxCapacity();
	size_t destEntityIndex = newIndex - srcChunkIndex * dest->maxCapacity();

	for (size_t i = 0; i < _components.size(); i++)
	{
		if (_components[i]->size() == 0)
			continue;
		if (source->_components.contains(_components[i]))
			dest->copyComponenet(src, srcEntityIndex, destEntityIndex, _components[i]);
	}
	_mutex.unlock();
	source->_mutex.unlock();
	return newIndex;
}

Chunk* Archetype::getChunk(size_t entity) const
{
	return _chunks[chunkIndex(entity)].get();
}

const size_t Archetype::entitySize()
{
	return _entitySize;
}

void Archetype::remove(size_t index)
{
	_mutex.lock();
	Chunk* chunk = getChunk(index);
	Chunk* lastChunk = _chunks[_chunks.size() - 1].get();
	size_t entityIndex = index - chunkIndex(index) * chunk->maxCapacity();

	for (size_t i = 0; i < _components.size(); i++)
	{
		chunk->copyComponenet(lastChunk, lastChunk->size() - 1, entityIndex, _components[i]);
	}
	
	lastChunk->removeEntity(lastChunk->size() - 1);

	if (lastChunk->size() == 0)
	{
		*_chunkAllocator << _chunks[_chunks.size() - 1];
		_chunks.resize(_chunks.size() - 1);
	}
	_size--;
	_mutex.unlock();
}

void Archetype::forEach(const ComponentSet& components, const std::function<void(const byte* [])>& f, size_t start, size_t end)
{
	_mutex.lock_shared();
	if (_chunks.size() == 0)
	{
		_mutex.unlock_shared();
		return;
	}
	assert(components.size() > 0);
	// Small stack vector allocations are ok in some circumstances, for instance if this were a regular ecs system this function would probably be a template and use the same amount of stack memory
	{
		const byte** data = (const byte**)STACK_ALLOCATE(sizeof(const byte*) * components.size());
		size_t* componentIndicies = (size_t*)STACK_ALLOCATE(sizeof(size_t) * components.size());
		size_t* componentSizes = (size_t*)STACK_ALLOCATE(sizeof(size_t) * components.size());

		for (size_t i = 0; i < components.size(); i++)
		{
			componentIndicies[i] = _chunks[0]->componentIndices()[_components.index(components[i])];
			componentSizes[i] = components[i]->size();
		}

		size_t startChunk = chunkIndex(start);
		size_t endChunk = chunkIndex(end);
		

		for (size_t chunk = startChunk; chunk <= endChunk; chunk++)
		{
			size_t lastIndex = std::min(_chunks[chunk]->size(), end - chunk * _chunks[0]->maxCapacity());
			_chunks[chunk]->lock_shared();
			for (size_t i = 0; i < lastIndex; i++)
			{
				for (size_t c = 0; c < components.size(); c++)
				{
					data[c] = _chunks[chunk]->data() + componentIndicies[c] + componentSizes[c] * i;
				}
				f(data);
			}
			_chunks[chunk]->unlock_shared();
		}
	}
	_mutex.unlock_shared();

}

void Archetype::forEach(const ComponentSet& components, const std::function<void(byte* [])>& f, size_t start, size_t end)
{
	_mutex.lock_shared();
	if (_chunks.size() == 0)
	{
		_mutex.unlock_shared();
		return;
	}
	assert(components.size() > 0);
	// Small stack vector allocations are ok in some circumstances, for instance if this were a regular ecs system this function would probably be a template and use the same amount of stack memory
	{
		byte** data = (byte**)STACK_ALLOCATE(sizeof(byte*) * components.size());
		size_t* componentIndicies = (size_t*)STACK_ALLOCATE(sizeof(size_t) * components.size());
		size_t* componentSizes = (size_t*)STACK_ALLOCATE(sizeof(size_t) * components.size());

		for (size_t i = 0; i < components.size(); i++)
		{
			componentIndicies[i] = _chunks[0]->componentIndices()[_components.index(components[i])];
			componentSizes[i] = components[i]->size();
		}

		size_t startChunk = chunkIndex(start);
		size_t endChunk = chunkIndex(end);

		for (size_t chunk = startChunk; chunk <= endChunk; chunk++)
		{
			size_t lastIndex = std::min(_chunks[chunk]->size(), end - chunk * _chunks[0]->maxCapacity());
			_chunks[chunk]->lock();
			for (size_t i = 0; i < _chunks[chunk]->size(); i++)
			{
				for (size_t c = 0; c < components.size(); c++)
				{
					data[c] = _chunks[chunk]->data() + componentIndicies[c] + componentSizes[c] * i;
				}
				f(data);
			}
			_chunks[chunk]->unlock();
		}
	}
	_mutex.unlock_shared();
}

ArchetypeEdge::ArchetypeEdge(const ComponentAsset* component, Archetype* archetype)
{
	assert(component && archetype);
	this->component = component;
	this->archetype = archetype;
}



Archetype* ArchetypeManager::makeArchetype(const ComponentSet& cdefs)
{
	_archetypeLock.lock();
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
						if (isRootArch[i] && connectingComponent != cdefs[i])
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
								componentRoots[i] = *(componentRoots.end() - 1);
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
	_archetypeLock.unlock();
	return newArch;
}

void ArchetypeManager::getRootArchetypes(const ComponentSet& components, std::vector<Archetype*>& roots) const
{
	_archetypeLock.lock_shared();
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
				assert(archetype->components().size() >= components.size());
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
					assert(archetype->components().size() >= components.size());
					roots.push_back(archetype);
					found.insert(archetype);
				}
			}
		}
	}
	_archetypeLock.unlock_shared();


}


void ArchetypeManager::updateForeachCache(const ComponentSet& components)
{
	_forEachLock.lock();
	for (ForEachData& data : _forEachData)
	{
		if (components.contains(data.components))
			data.cached = false;
	}
	_forEachLock.unlock();
}



std::vector<Archetype*>& ArchetypeManager::getForEachArchetypes(EnityForEachID id)
{
	assert(_forEachData.size() > id);
	// If the list is more then
	if (!_forEachData[id].cached)
	{
		//create new cache
		_forEachLock.lock();
		getRootArchetypes(_forEachData[id].components, _forEachData[id].archetypeRoots);
		_forEachData[id].cached = true;
		_forEachLock.unlock();
	}
	return _forEachData[id].archetypeRoots;
}


size_t ArchetypeManager::forEachCount(EnityForEachID id)
{
	_forEachLock.lock_shared();
	size_t count = 0;
	for (Archetype* archetype : getForEachArchetypes(id))
	{
		count += archetype->size();
	}
	_forEachLock.unlock_shared();
	return count;
}

void ArchetypeManager::forEach(EnityForEachID id, const std::function<void(byte* [])>& f)
{
	std::unordered_set<Archetype*> executed;
	_archetypeLock.lock_shared();
	_forEachLock.lock_shared();
	assert(id < _forEachData.size());

	for (Archetype* archetype : getForEachArchetypes(id))
	{
		forEachRecursive(archetype, _forEachData[id].components, f, executed);
	}

	_archetypeLock.unlock_shared();
	_forEachLock.unlock_shared();
}

void ArchetypeManager::constForEach(EnityForEachID id, const std::function<void(const byte* [])>& f)
{
	std::unordered_set<Archetype*> executed;
	_archetypeLock.lock_shared();
	_forEachLock.lock_shared();
	assert(id < _forEachData.size());

	for (Archetype* archetype : getForEachArchetypes(id))
	{
		forEachRecursive(archetype, _forEachData[id].components, f, executed);
	}

	_archetypeLock.unlock_shared();
	_forEachLock.unlock_shared();
}

std::shared_ptr<JobHandle> ArchetypeManager::constForEachParellel(EnityForEachID id, const std::function <void(const byte* [])>& f, size_t entitiesPerThread)
{
	std::unordered_set<Archetype*> executed;
	std::shared_ptr<JobHandle> handle = std::make_shared<JobHandle>();
	_archetypeLock.lock_shared();
	_forEachLock.lock_shared();
	assert(id < _forEachData.size());

	for (Archetype* archetype : getForEachArchetypes(id))
	{
		forEachRecursiveParellel(archetype, _forEachData[id].components, f, executed, entitiesPerThread, handle);
	}

	_archetypeLock.unlock_shared();
	_forEachLock.unlock_shared();
	return handle;
}

std::shared_ptr<JobHandle> ArchetypeManager::forEachParellel(EnityForEachID id, const std::function <void(byte* [])>& f, size_t entitiesPerThread)
{
	std::unordered_set<Archetype*> executed;
	std::shared_ptr<JobHandle> handle = std::make_shared<JobHandle>();
	_archetypeLock.lock_shared();
	_forEachLock.lock_shared();
	assert(id < _forEachData.size());

	for (Archetype* archetype : getForEachArchetypes(id))
	{
		forEachRecursiveParellel(archetype, _forEachData[id].components, f, executed, entitiesPerThread, handle);
	}

	_archetypeLock.unlock_shared();
	_forEachLock.unlock_shared();
	return handle;
}

ArchetypeManager::ArchetypeManager()
{
	_chunkAllocator = std::make_shared<ChunkPool>();
}

Archetype* ArchetypeManager::getArchetype(const ComponentSet& components)
{
	_archetypeLock.lock_shared();
	size_t numComps = components.size();
	assert(numComps > 0);
	if (numComps > _archetypes.size())
	{
		_archetypeLock.unlock_shared();
		return makeArchetype(components);
	}
	auto& archetypes = _archetypes[numComps - 1];

	for (size_t a = 0; a < archetypes.size(); a++)
	{
		assert(archetypes[a]->components().size() == components.size());
		if (archetypes[a]->hasComponents(components))
		{
			_archetypeLock.unlock_shared();
			return archetypes[a].get();
		}
	}
	_archetypeLock.unlock_shared();
	return makeArchetype(components);
}
EnityForEachID ArchetypeManager::getForEachID(const ComponentSet& components)
{
	//std::sort(components.begin(), components.end());
	_forEachLock.lock();
	EnityForEachID id = 0;
	for (ForEachData& d : _forEachData)
	{
		if (d.components.size() == components.size() && d.components.contains(components))
		{
			_forEachLock.unlock();
			return id;
		}
		++id;
	}
	//If we reatch here there is no prexisting cache
	_forEachData.push_back(ForEachData(components));
	_forEachLock.unlock();
	return id;
}

ArchetypeManager::ForEachData::ForEachData(ComponentSet components)
{
	this->components = std::move(components);
	cached = false;
}
