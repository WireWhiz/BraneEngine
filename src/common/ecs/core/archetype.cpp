#include "archetype.h"

#include "chunk.h"

ArchetypeView::ArchetypeView(Archetype& archetype, const ComponentSet& components) : _arch{&archetype}, componentOffsets(components.size())
{
	descriptions.reserve(components.size());
	archetype._components.indices(components, componentOffsets);
	for(auto o : componentOffsets)
		descriptions.push_back(_arch->_componentDescriptions[o]);
	for (size_t i = 0; i < components.size(); i++)
	{
		componentSizes[i] = descriptions[i]->size();
	}
}

ArchetypeView::ArchetypeView()
{

}

ArchetypeView::iterator::iterator(ArchetypeView& view, size_t chunkIndex, size_t entityIndex) : _view(view), _chunkIndex(chunkIndex), _entityIndex(entityIndex)
{
	components.resize(_view.descriptions.size(), nullptr);
	if(_chunkIndex < _view._arch->_chunks.size())
	{
		for (size_t i = 0; i < components.size(); ++i)
		{
			components[i] = _view._arch->_chunks[_chunkIndex]->getComponentData(_view.descriptions[i], _entityIndex);
		}
	}
}

void ArchetypeView::iterator::operator++()
{
	_entityIndex ++;
	if(_entityIndex >= _view._arch->_chunks[_chunkIndex]->maxCapacity())
	{
		_entityIndex = 0;
		_chunkIndex++;
	}
	if(_chunkIndex < _view._arch->_chunks.size())
	{
		for (size_t i = 0; i < components.size(); ++i)
		{
			components[i] = _view._arch->_chunks[_chunkIndex]->getComponentData(_view.descriptions[i], _entityIndex);
		}
	}
}

bool ArchetypeView::iterator::operator!=(const ArchetypeView::iterator& o) const
{
	return !(o == *this);
}

bool ArchetypeView::iterator::operator==(const ArchetypeView::iterator& o) const
{
	return _chunkIndex == o._chunkIndex && _entityIndex == o._entityIndex;
}

const std::vector<byte*>& ArchetypeView::iterator::operator*() const
{
	return components;
}

ArchetypeView::iterator ArchetypeView::begin()
{
	assert(_arch);
	return iterator(*this, 0, 0);
}

ArchetypeView::iterator ArchetypeView::end()
{
	assert(_arch);
	return iterator(*this, _arch->_chunks.size(), 0);
}

size_t Archetype::chunkIndex(size_t entity) const
{
	return entity / (Chunk::allocationSize() / _entitySize);
}

Archetype::Archetype(const std::vector<const ComponentDescription*>& components, std::shared_ptr<ChunkPool>& chunkAllocator)
{
	_chunkAllocator = chunkAllocator;
	_entitySize = 0;
	_componentDescriptions = components;
	for (auto component : components)
	{
		_components.add(component->id);
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

bool Archetype::hasComponent(ComponentID component) const
{
	return _components.contains(component);
}

bool Archetype::hasComponents(const ComponentSet& comps) const
{
	return _components.contains(comps);
}

VirtualComponent Archetype::getComponent(size_t entity, ComponentID component) const
{
	_mutex.lock_shared();
	assert(_components.contains(component));

	size_t chunk = chunkIndex(entity);
	assert(chunk < _chunks.size());

	size_t index = entity - chunk * _chunks[0]->maxCapacity();
	assert(index < _chunks[chunk]->size());
	VirtualComponent o = _chunks[chunk]->getComponent(_componentDescriptions[_components.index(component)], index);
	_mutex.unlock_shared();
	return o;
}

void Archetype::setComponent(size_t entity, const VirtualComponent& component)
{
	_mutex.lock();
	assert(_components.contains(component.description()->id));

	size_t chunk = chunkIndex(entity);
	assert(chunk < _chunks.size());

	size_t index = entity - chunk * _chunks[0]->maxCapacity();
	assert(index < _chunks[chunk]->size());
	_chunks[chunk]->setComponent(component, index);
	_mutex.unlock();
}

void Archetype::setComponent(size_t entity, const VirtualComponentView& component)
{
	_mutex.lock();
	assert(_components.contains(component.description()->id));

	size_t chunk = chunkIndex(entity);
	assert(chunk < _chunks.size());

	size_t index = entity - chunk * _chunks[0]->maxCapacity();
	assert(index < _chunks[chunk]->size());
	_chunks[chunk]->setComponent(component, index);
	_mutex.unlock();
}

bool Archetype::isChildOf(const Archetype* parent, ComponentID& connectingComponent) const
{
	connectingComponent = -1;
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
	if(connectingComponent == (ComponentID)-1)
		connectingComponent = parent->_components[parent->_components.size() - 1];
	_mutex.unlock_shared();
	return true;
}

const ComponentSet& Archetype::components() const
{
	return _components;
}

const std::vector<const ComponentDescription*>& Archetype::componentDescriptions()
{
	return _componentDescriptions;
}

std::shared_ptr<ArchetypeEdge> Archetype::getAddEdge(ComponentID component)
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

std::shared_ptr<ArchetypeEdge> Archetype::getRemoveEdge(ComponentID component)
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

void Archetype::addAddEdge(ComponentID component, Archetype* archetype)
{
	ASSERT_MAIN_THREAD();
	_addEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
}

void Archetype::addRemoveEdge(ComponentID component, Archetype* archetype)
{
	ASSERT_MAIN_THREAD();
	_removeEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
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

	for (auto _component : _componentDescriptions)
	{
		if (_component->size() == 0)
			continue;
		if (source->_components.contains(_component->id))
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
		chunk->copyComponenet(lastChunk, lastChunk->size() - 1, entityIndex, _componentDescriptions[i]);
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

void Archetype::forEach(const std::vector<ComponentID>& components, const std::function<void(const byte* [])>& f, size_t start, size_t end)
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
			size_t componentIndex = _components.index(components[i]);
            componentIndices[i] = _chunks[0]->componentIndices()[componentIndex];
			componentSizes[i] = _componentDescriptions[componentIndex]->size();;
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

void Archetype::forEach(const std::vector<ComponentID>& components, const std::function<void(byte* [])>& f, size_t start, size_t end)
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
			size_t componentIndex = _components.index(components[i]);
            componentIndices[i] = _chunks[0]->componentIndices()[componentIndex];
			componentSizes[i] = _componentDescriptions[componentIndex]->size();
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

ArchetypeView Archetype::getComponents(const ComponentSet& components)
{
	return ArchetypeView(*this, components);
}

ArchetypeEdge::ArchetypeEdge(ComponentID component, Archetype* archetype)
{
	assert(component && archetype);
	this->component = component;
	this->archetype = archetype;
}

ArchetypeSet::iterator::iterator(ArchetypeSet& set, size_t index) : _set(set)
{
	_archetypeIndex = index;
}

void ArchetypeSet::iterator::operator++()
{
	++_archetypeIndex;
}

bool ArchetypeSet::iterator::operator!=(const ArchetypeSet::iterator& o) const
{
	return !(o == *this);
}

bool ArchetypeSet::iterator::operator==(const ArchetypeSet::iterator& o) const
{
	return _archetypeIndex == o._archetypeIndex;
}

Archetype& ArchetypeSet::iterator::operator*()
{
	return *_set._archetypes[_archetypeIndex];
}

ArchetypeSet::ArchetypeSet(ArchetypeManager& manager, const ComponentSet& components) : _manager(manager)
{
	if(components.size() == 0)
		return;

	for(size_t compCount = components.size() -1; compCount < _manager._archetypes.size(); ++compCount)
	{
		for(auto& archetype : _manager._archetypes[compCount])
		{
			if(archetype->hasComponents(components))
				_archetypes.push_back(archetype.get());
		}
	}
}

ArchetypeSet::iterator ArchetypeSet::begin()
{
	return ArchetypeSet::iterator(*this, 0);
}

ArchetypeSet::iterator ArchetypeSet::end()
{
	return ArchetypeSet::iterator(*this, _archetypes.size());
}

ArchetypeManager::ArchetypeManager(ComponentManager& componentManager) : _componentManager(componentManager)
{
	_chunkAllocator = std::make_shared<ChunkPool>();
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

	std::vector<const ComponentDescription*> descriptions;
	descriptions.reserve(components.size());
	for(auto id : components)
		descriptions.push_back(_componentManager.getComponent(id));

	size_t newIndex = _archetypes[numComps - 1].size();
	_archetypes[numComps - 1].push_back(std::make_unique<Archetype>(descriptions, _chunkAllocator));

	Archetype* newArch = _archetypes[numComps - 1][newIndex].get();



	{//Scope for bool array
		// find edges to other archetypes lower than this one
		if (numComps > 1)
		{
			ComponentID connectingComponent;
			for (size_t i = 0; i < _archetypes[numComps - 2].size(); i++)
			{
				if (_archetypes[numComps - 2][i]->isChildOf(newArch, connectingComponent))
				{
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
				ComponentID connectingComponent;
				if (newArch->isChildOf(_archetypes[numComps][i].get(), connectingComponent))
				{
					Archetype* otherArch = _archetypes[numComps][i].get();
					newArch->addAddEdge(connectingComponent, otherArch);
					otherArch->addRemoveEdge(connectingComponent, newArch);
				}
			}
		}
	}
	return newArch;
}

void ArchetypeManager::forEach(const std::vector<ComponentID>& components, const std::function<void(byte* [])>& f)
{
	ASSERT_MAIN_THREAD();
	std::unordered_set<Archetype*> executed;
	ArchetypeSet archetypes = getArchetypes(components);
	for (Archetype& archetype : archetypes)
	{
		archetype.forEach(components, f, 0, archetype.size());
	}

}

void ArchetypeManager::constForEach(const std::vector<ComponentID>& components, const std::function<void(const byte* [])>& f)
{
	ASSERT_MAIN_THREAD();

	ArchetypeSet archetypes = getArchetypes(components);
	for (Archetype& archetype : archetypes)
	{
		archetype.forEach(components, f, 0, archetype.size());
	}

}

std::shared_ptr<JobHandle> ArchetypeManager::constForEachParallel(const std::vector<ComponentID>& components, const std::function <void(const byte* [])>& f, size_t entitiesPerThread)
{
	ASSERT_MAIN_THREAD();
	std::shared_ptr<JobHandle> handle = std::make_shared<JobHandle>();

	ArchetypeSet archetypes = getArchetypes(components);
	for (Archetype& archetype : archetypes)
	{
		size_t threads = std::max<size_t>(archetype.size() / entitiesPerThread, 1);

		for (size_t t = 0; t < threads; t++)
		{
			size_t start = t * entitiesPerThread;
			size_t end = std::min(t * entitiesPerThread + entitiesPerThread, archetype.size());
			ThreadPool::enqueue([&archetype, &f, components, start, end]() {
				archetype.forEach(components, f, start, end);
			}, handle);
		}
	}

	return handle;
}

std::shared_ptr<JobHandle> ArchetypeManager::forEachParallel(const std::vector<ComponentID>& components, const std::function <void(byte* [])>& f, size_t entitiesPerThread)
{
	ASSERT_MAIN_THREAD();
	std::unordered_set<Archetype*> executed;
	std::shared_ptr<JobHandle> handle = std::make_shared<JobHandle>();
	ArchetypeSet archetypes = getArchetypes(components);
	for (Archetype& archetype : archetypes)
	{
		size_t threads = std::max<size_t>(archetype.size() / entitiesPerThread, 1);

		for (size_t t = 0; t < threads; t++)
		{
			size_t start = t * entitiesPerThread;
			size_t end = std::min(t * entitiesPerThread + entitiesPerThread, archetype.size());
			ThreadPool::enqueue([&archetype, &f, &components, start, end]() {
				archetype.forEach(components, f, start, end);
			}, handle);
		}
	}

	return handle;
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

void ArchetypeManager::clear()
{
	_archetypes.resize(0);
}

ArchetypeSet ArchetypeManager::getArchetypes(const ComponentSet& components)
{
	return ArchetypeSet(*this, components);
}
