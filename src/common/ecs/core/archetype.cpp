#include "archetype.h"

#include "chunk.h"


size_t Archetype::chunkIndex(size_t entity) const
{
	return entity / (Chunk::allocationSize() / _entitySize);
}

Archetype::Archetype(const ComponentSet& components, std::shared_ptr<ChunkPool>& chunkAllocator) : _components(components)
{
	_chunkAllocator = chunkAllocator;
	_entitySize = 0;
	for (auto component : components)
	{
		_entitySize += component->size();
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
	assert(_components.size() + 1 == parent->_components.size()); //Make sure this is a valid comparison
	byte missCount = 0;
	for (size_t i = 0; i - missCount < _components.size(); i++)
	{
		assert(i - missCount < _components.size());
			
		if (_components[i - missCount] != parent->_components[i])
		{
			connectingComponent = parent->_components[i];
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

const ComponentSet& Archetype::components() const
{
	return _components;
}

std::shared_ptr<ArchetypeEdge> Archetype::getAddEdge(const ComponentAsset* component)
{
	ASSERT_MAIN_THREAD();
	for (auto & _addEdge : _addEdges)
	{
		if (component == _addEdge->component)
		{
			return _addEdge;
		}
	}
	return nullptr;
}

std::shared_ptr<ArchetypeEdge> Archetype::getRemoveEdge(const ComponentAsset* component)
{
	ASSERT_MAIN_THREAD();
	for (auto & _removeEdge : _removeEdges)
	{
		if (component == _removeEdge->component)
		{
			return _removeEdge;
		}
	}
	return nullptr;
}

void Archetype::addAddEdge(const ComponentAsset* component, Archetype* archetype)
{
	ASSERT_MAIN_THREAD();
	_addEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
}

void Archetype::addRemoveEdge(const ComponentAsset* component, Archetype* archetype)
{
	ASSERT_MAIN_THREAD();
	_removeEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
}

void Archetype::forAddEdge(const std::function<void(std::shared_ptr<const ArchetypeEdge>)>& f) const
{
	ASSERT_MAIN_THREAD();
	for (const auto & _addEdge : _addEdges)
	{
		f(_addEdge);
	}
}

void Archetype::forRemoveEdge(std::function<void(std::shared_ptr<const ArchetypeEdge>)>& f) const
{
	ASSERT_MAIN_THREAD();
	for (const auto & _removeEdge : _removeEdges)
	{
		f(_removeEdge);
	}
}

size_t Archetype::size() const
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

	for (auto _component : _components)
	{
		if (_component->size() == 0)
			continue;
		if (source->_components.contains(_component))
			dest->copyComponenet(src, srcEntityIndex, destEntityIndex, _component);
	}
	_mutex.unlock();
	source->_mutex.unlock();
	return newIndex;
}

Chunk* Archetype::getChunk(size_t entity) const
{
	return _chunks[chunkIndex(entity)].get();
}

size_t Archetype::entitySize() const
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
		size_t* componentIndices = (size_t*)STACK_ALLOCATE(sizeof(size_t) * components.size());
		size_t* componentSizes = (size_t*)STACK_ALLOCATE(sizeof(size_t) * components.size());

		for (size_t i = 0; i < components.size(); i++)
		{
            componentIndices[i] = _chunks[0]->componentIndices()[_components.index(components[i])];
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
					data[c] = _chunks[chunk]->data() + componentIndices[c] + componentSizes[c] * i;
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
		size_t* componentIndices = (size_t*)STACK_ALLOCATE(sizeof(size_t) * components.size());
		size_t* componentSizes = (size_t*)STACK_ALLOCATE(sizeof(size_t) * components.size());

		for (size_t i = 0; i < components.size(); i++)
		{
            componentIndices[i] = _chunks[0]->componentIndices()[_components.index(components[i])];
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
					data[c] = _chunks[chunk]->data() + componentIndices[c] + componentSizes[c] * i;
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



Archetype* ArchetypeManager::makeArchetype(const ComponentSet& components)
{
	ASSERT_MAIN_THREAD();
	assert(components.size() > 0);
	size_t numComps = components.size();

	// We want to keep archetypes of the same size in groups
	// This means we keep each size in a different vector

	// Make sure we have enough vectors
	if (numComps > _archetypes.size())
		_archetypes.resize(numComps);

	size_t newIndex = _archetypes[numComps - 1].size();
	_archetypes[numComps - 1].push_back(std::make_unique<Archetype>(components, _chunkAllocator));

	Archetype* newArch = _archetypes[numComps - 1][newIndex].get();



	{//Scope for bool array
		// find edges to other archetypes lower than this one
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
				}
			}
		}

		// find edges to other archetypes higher than this one
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
				}
			}
		}
	}
	cacheArchetype(newArch);
	return newArch;
}

void ArchetypeManager::findArchetypes(const ComponentSet& components, const ComponentSet& exclude, std::vector<Archetype*>& archetypes) const
{
	ASSERT_MAIN_THREAD();
	archetypes.clear();
	// Search through any archetypes of the exact size that we want to see if there's one that fits
	if (_archetypes.size() >= components.size())
	{
		// Searches through all archetypes with the same number of components as the for each
		for (size_t i = components.size() - 1; i < _archetypes.size(); i++)
		{
			for (size_t a = 0; a < _archetypes[i].size(); a++)
			{
				if (_archetypes[i][a]->hasComponents(components) && (exclude.size() == 0 || !_archetypes[i][a]->hasComponents(exclude)))
				{
					assert(_archetypes[i][a]->components().size() >= components.size());
					archetypes.push_back(_archetypes[i][a].get());
				}
			}
		}
	}


}

void ArchetypeManager::cacheArchetype(Archetype* arch)
{
	ASSERT_MAIN_THREAD();
	for (ForEachData& data : _forEachData)
	{
		if (arch->components().contains(data.components) && !arch->components().contains(data.exclude))
			data.archetypes.push_back(arch);
	}
}

void ArchetypeManager::removeCachedArchetype(Archetype* arch)
{
	ASSERT_MAIN_THREAD();
	for (ForEachData& data : _forEachData)
	{
		if (arch->components().contains(data.components) && !arch->components().contains(data.exclude))
			for (size_t i = 0; i < data.archetypes.size(); i++)
			{
				data.archetypes.erase(data.archetypes.begin() + i);
			}
	}

}



std::vector<Archetype*>& ArchetypeManager::getForEachArchetypes(EntityForEachID id)
{
	ASSERT_MAIN_THREAD();
	assert(_forEachData.size() > id);
	return _forEachData[id].archetypes;
}


size_t ArchetypeManager::forEachCount(EntityForEachID id)
{
	ASSERT_MAIN_THREAD();
	size_t count = 0;
	for (Archetype* archetype : getForEachArchetypes(id))
	{
		count += archetype->size();
	}
	return count;
}

void ArchetypeManager::forEach(EntityForEachID id, const std::function<void(byte* [])>& f)
{
	ASSERT_MAIN_THREAD();
	std::unordered_set<Archetype*> executed;
	assert(id < _forEachData.size());

	for (Archetype* archetype : getForEachArchetypes(id))
	{
		archetype->forEach(_forEachData[id].components, f, 0, archetype->size());
	}

}

void ArchetypeManager::constForEach(EntityForEachID id, const std::function<void(const byte* [])>& f)
{
	ASSERT_MAIN_THREAD();
	std::unordered_set<Archetype*> executed;
	assert(id < _forEachData.size());

	for (Archetype* archetype : getForEachArchetypes(id))
	{
		archetype->forEach(_forEachData[id].components, f, 0, archetype->size());
	}

}

std::shared_ptr<JobHandle> ArchetypeManager::constForEachParallel(EntityForEachID id, const std::function <void(const byte* [])>& f, size_t entitiesPerThread)
{
	ASSERT_MAIN_THREAD();
	std::unordered_set<Archetype*> executed;
	std::shared_ptr<JobHandle> handle = std::make_shared<JobHandle>();
	assert(id < _forEachData.size());
	ComponentSet& components = _forEachData[id].components;

	for (Archetype* archetype : getForEachArchetypes(id))
	{
		size_t threads = archetype->size() / entitiesPerThread + 1;

		for (size_t t = 0; t < threads; t++)
		{
			size_t start = t * entitiesPerThread;
			size_t end = std::min(t * entitiesPerThread + entitiesPerThread, archetype->size());
			ThreadPool::enqueue([archetype, &f, &components, start, end]() {
				archetype->forEach(components, f, start, end);
			}, handle);
		}
	}

	return handle;
}

std::shared_ptr<JobHandle> ArchetypeManager::forEachParallel(EntityForEachID id, const std::function <void(byte* [])>& f, size_t entitiesPerThread)
{
	ASSERT_MAIN_THREAD();
	std::unordered_set<Archetype*> executed;
	std::shared_ptr<JobHandle> handle = std::make_shared<JobHandle>();
	assert(id < _forEachData.size());
	ComponentSet& components = _forEachData[id].components;

	for (Archetype* archetype : getForEachArchetypes(id))
	{
		size_t threads = archetype->size() / entitiesPerThread + 1;

		for (size_t t = 0; t < threads; t++)
		{
			size_t start = t * entitiesPerThread;
			size_t end = std::min(t * entitiesPerThread + entitiesPerThread, archetype->size());
			ThreadPool::enqueue([archetype, &f, &components, start, end]() {
				archetype->forEach(components, f, start, end);
			}, handle);
		}
	}

	return handle;
}

ArchetypeManager::ArchetypeManager()
{
	_chunkAllocator = std::make_shared<ChunkPool>();
}

Archetype* ArchetypeManager::getArchetype(const ComponentSet& components)
{
	ASSERT_MAIN_THREAD();
	size_t numComps = components.size();
	assert(numComps > 0);
	if (numComps > _archetypes.size())
	{
		return makeArchetype(components);
	}
	auto& archetypes = _archetypes[numComps - 1];

	for (size_t a = 0; a < archetypes.size(); a++)
	{
		assert(archetypes[a]->components().size() == components.size());
		if (archetypes[a]->hasComponents(components))
		{
			return archetypes[a].get();
		}
	}
	return makeArchetype(components);
}
EntityForEachID ArchetypeManager::getForEachID(const ComponentSet& components, const ComponentSet& exclude)
{
	ASSERT_MAIN_THREAD();
	//std::sort(components.begin(), components.end());
	EntityForEachID id = 0;
	for (ForEachData& d : _forEachData)
	{
		if (d.components.size() == components.size() && d.components.contains(components))
		{
			return id;
		}
		++id;
	}

	//If we reattach here there is no preexisting cache
	_forEachData.emplace_back(components, exclude);
	assert(id == _forEachData.size() - 1);
	findArchetypes(_forEachData[id].components, _forEachData[id].exclude, _forEachData[id].archetypes);
	return id;
}

ArchetypeManager::ForEachData::ForEachData(ComponentSet components, ComponentSet exclude)
{
	this->components = std::move(components);
	this->exclude = std::move(exclude);
}
