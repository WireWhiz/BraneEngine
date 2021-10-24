#pragma once
#include <array>
#include "virtualType.h"
#include "archetype.h"
#include <algorithm>
#include <functional>

template<size_t N>
class ChunkBase
{
#ifdef TEST_BUILD
public:
#endif
	size_t _size;
	std::array<byte, N> _data;
	Archetype* _archetype;
	std::vector<size_t> _componentIndices;

public:
	void setArchetype(Archetype* archetype)
	{
		clear();
		_archetype = archetype;

		_componentIndices = std::vector<size_t>(_archetype->components().size());
		for (size_t i = 0; i < _archetype->components().size(); i++)
		{
			if (i != 0)
				_componentIndices[i] = _componentIndices[i - 1] + _archetype->components()[i - 1]->size() * maxCapacity();
		}
	}

	byte* getComponent(size_t component, size_t index)
	{
		return &_data[_componentIndices[component] + index * _archetype->components()[component]->size()];
	}

	size_t createEntity()
	{
		assert(_size < maxCapacity());
		size_t index = _size;
		for (size_t c = 0; c < _archetype->components().size(); c++)
		{
			if(_archetype->components()[c]->size() != 0)
				_archetype->components()[c]->construct(getComponent(c, index));
		}
		++_size;
		return index;
	}

	void copyComponenet(ChunkBase* source, size_t sIndex, size_t dIndex, const ComponentAsset* component)
	{
		assert(source->_archetype->components().contains(component));
		assert(_archetype->components().contains(component));
		size_t sourceCompIndex = source->_archetype->components().index(component);
		size_t destCompIndex = _archetype->components().index(component);
		assert(sIndex + source->_componentIndices[sourceCompIndex] + component->size() < N);
		assert(dIndex + _componentIndices[destCompIndex] + component->size() < N);
		size_t src = sIndex + source->_componentIndices[sourceCompIndex];
		size_t dest = dIndex + _componentIndices[destCompIndex];
		std::copy(&source->_data[src], &source->_data[src + component->size()], &_data[dest]);
	}
	
	void removeEntity(size_t index)
	{
		assert(_size > 0);
		assert(index < _size);

		// Deconstruct the components of the removed entity
		for (size_t c = 0; c < _archetype->components().size(); c++)
		{
			if (_archetype->components()[c]->size() != 0)
				_archetype->components()[c]->deconstruct(getComponent(c, index));
		}

		//Copy last index to the one that we are removing
		if (index != _size - 1)
		{
			for (size_t i = 0; i < _archetype->components().size(); i++)
			{
				if (_archetype->components()[i]->size() != 0)
					std::copy(getComponent(i, _size - 1), getComponent(i, _size - 1), getComponent(i, index));
			}
		}

		//Remove the last index
		--_size;
		
	}

	void clear()
	{
		if (_archetype)
		{
			for (size_t c = 0; c < _archetype->components().size(); c++)
			{
				for (size_t i = 0; i < _size; i++)
				{
					_archetype->components()[c]->deconstruct(getComponent(c, i));
				}
			}
		}
		_data.fill(0);
		_size = 0;
	}

	size_t size()
	{
		return _size;
	}

	size_t maxCapacity()
	{
		return N / _archetype->entitySize();
	}

	const std::vector<size_t>& componentIndices()
	{
		return _componentIndices;
	}

	byte* data()
	{
		return _data.data();
	}

	static size_t allocationSize()
	{
		return N;
	}
};

typedef ChunkBase<16384> Chunk;



class ChunkPool
{
#ifdef TEST_BUILD
public:
#endif
	std::vector<std::unique_ptr<Chunk>> _unused;

public:
	friend void operator>>(ChunkPool& pool, std::unique_ptr<Chunk>& dest);
	friend void operator<<(ChunkPool& pool, std::unique_ptr<Chunk>& dest);
};