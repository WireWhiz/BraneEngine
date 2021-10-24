#include "Archetype.h"

#include "chunk.h"


size_t Archetype::chunkIndex(size_t entity) const
{
	return entity / (Chunk::allocationSize() / _entitySize);
}

Archetype::Archetype(const ComponentSet& components, ChunkPool& chunkAllocator) : _components(components), _chunkAllocator(chunkAllocator)
{
	//_components.reserve(componentDefs.size());
	_entitySize = 0;
	for (size_t i = 0; i < components.size(); i++)
	{
		//_components.push_back(VirtualComponentVector(_componentDefs[i]));
		_entitySize += components[i]->size();
	}

}

Archetype::~Archetype()
{
	while (!_chunks.empty())
	{
		_chunkAllocator << _chunks[_chunks.size() - 1];
		_chunks.resize(_chunks.size() - 1);
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

const VirtualComponentPtr Archetype::getComponent(size_t entity, const ComponentAsset* component) const
{
	assert(_components.contains(component));
	return VirtualComponentPtr(component, getComponent(entity, _components.index(component)));
}

byte* Archetype::getComponent(size_t entity, size_t component) const
{
	assert(component < _components.size());

	size_t chunk = chunkIndex(entity);
	assert(chunk < _chunks.size());

	size_t index = entity - chunk * _chunks[0]->maxCapacity();
	assert(index < _chunks[chunk]->size());
	return _chunks[chunk]->getComponent(component, index);
}

bool Archetype::isChildOf(const Archetype* parent, const ComponentAsset*& connectingComponent) const
{
	assert(_components.size() + 1 == parent->_components.size()); //Make sure this is a valid comparason
	byte missCount = 0;
	for (size_t i = 0; i - missCount < _components.size(); i++)
	{
		assert(i - missCount < _components.size());
			
		if (_components[i - missCount] != parent->_components[i])
		{
			connectingComponent = parent->_components.components()[i];
			if (++missCount > 1)
				return false;
		}
	}
	if(!connectingComponent)
		connectingComponent = parent->_components[parent->_components.size() - 1];
	return true;
}

bool Archetype::isRootForComponent(const ComponentAsset* component) const
{
	if (_components.size() == 1)
		return true;
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		if (_removeEdges[i]->component == component)
			return true;
	}
	return false;
}


const ComponentSet& Archetype::components() const
{
	return _components;
}

std::shared_ptr<ArchetypeEdge> Archetype::getAddEdge(const ComponentAsset* component)
{
	for (size_t i = 0; i < _addEdges.size(); i++)
	{
		if (component == _addEdges[i]->component)
			return _addEdges[i];
	}
	return nullptr;
}

std::shared_ptr<ArchetypeEdge> Archetype::getRemoveEdge(const ComponentAsset* component)
{
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		if (component == _removeEdges[i]->component)
			return _removeEdges[i];
	}
	return std::shared_ptr<ArchetypeEdge>();
}

void Archetype::addAddEdge(const ComponentAsset* component, Archetype* archetype)
{
	_addEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
}

void Archetype::addRemoveEdge(const ComponentAsset* component, Archetype* archetype)
{
	_removeEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
}

void Archetype::forAddEdge(const std::function<void(std::shared_ptr<ArchetypeEdge>)>& f)
{
	for (size_t i = 0; i < _addEdges.size(); i++)
	{
		f(_addEdges[i]);
	}
}

void Archetype::forRemoveEdge(std::function<void(std::shared_ptr<ArchetypeEdge>)>& f)
{
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		f(_removeEdges[i]);
	}
}

size_t Archetype::size()
{
	return _size;
}

size_t Archetype::createEntity()
{
	size_t chunk = chunkIndex(_size);
	if (chunk >= _chunks.size())
	{
		_chunks.resize(_chunks.size() + 1);
		_chunkAllocator >> _chunks[_chunks.size() - 1];
		_chunks[_chunks.size() - 1]->setArchetype(this);
	}

	assert(chunk < _chunks.size());
	_chunks[chunk]->createEntity();

	++_size;
	return _size - 1;
}

size_t Archetype::copyEntity(Archetype* source, size_t index)
{
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
		_chunkAllocator << _chunks[_chunks.size() - 1];
		_chunks.resize(_chunks.size() - 1);
	}
		


	_size--;
}

void Archetype::forEach(const ComponentSet& components, const std::function<void(byte* [])>& f)
{
	if (_chunks.size() == 0)
		return;
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
		
		for (size_t chunk = 0; chunk < _chunks.size(); chunk++)
		{
			for (size_t i = 0; i < _chunks[chunk]->size(); i++)
			{
				for (size_t c = 0; c < components.size(); c++)
				{
					data[c] = _chunks[chunk]->data() + componentIndicies[c] + componentSizes[c] * i;
				}
				f(data);
			}
		}
	}
	
}

ArchetypeEdge::ArchetypeEdge(const ComponentAsset* component, Archetype* archetype)
{
	assert(component && archetype);
	this->component = component;
	this->archetype = archetype;
}
