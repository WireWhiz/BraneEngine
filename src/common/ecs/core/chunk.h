#pragma once
#include <array>
#include <vector>
#include "virtualType.h"
#include "archetype.h"
#include <algorithm>
#include <functional>
#include <utility/shared_recursive_mutex.h>

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
	shared_recursive_mutex _lock;
public:
	ChunkBase()
	{
		_size = 0;
		_archetype = nullptr;
	}
	ChunkBase(const ChunkBase&) = delete;
	~ChunkBase()
	{
		clear();
	}
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

	byte* getComponentData(const ComponentAsset* component, size_t entity)
	{

		return &_data[_componentIndices[_archetype->components().index(component)] + entity * component->size()];
	}
	
	byte* getComponentData(size_t component, size_t entity)
	{

		return &_data[_componentIndices[component] + entity * _archetype->components()[component]->size()];
	}

	VirtualComponent getComponent(const ComponentAsset* component, size_t entity)
	{
		_lock.lock_shared();
		VirtualComponent o = VirtualComponent(component, getComponentData(component, entity));
		_lock.unlock_shared();
		return std::move(o);
	}

	void setComponent(const VirtualComponent& component, size_t entity)
	{
		_lock.lock();
		std::copy(component.data(), component.data() + component.def()->size(), getComponentData(_archetype->components().index(component.def()), entity));
		_lock.unlock();
	}

	size_t createEntity()
	{
		_lock.lock();
		assert(_size < maxCapacity());
		size_t index = _size;
		for (size_t c = 0; c < _archetype->components().size(); c++)
		{
			if(_archetype->components()[c]->size() != 0)
				_archetype->components()[c]->construct(getComponentData(c, index));
		}
		++_size;
		_lock.unlock();
		return index;
	}

	void copyComponenet(ChunkBase* source, size_t sIndex, size_t dIndex, const ComponentAsset* component)
	{
		_lock.lock();
		source->_lock.lock();
		assert(source->_archetype->components().contains(component));
		assert(_archetype->components().contains(component));
		size_t sourceCompIndex = source->_archetype->components().index(component);
		size_t destCompIndex = _archetype->components().index(component);
		assert(sIndex + source->_componentIndices[sourceCompIndex] + component->size() < N);
		assert(dIndex + _componentIndices[destCompIndex] + component->size() < N);
		size_t src = sIndex + source->_componentIndices[sourceCompIndex];
		size_t dest = dIndex + _componentIndices[destCompIndex];
		std::copy(&source->_data[src], &source->_data[src + component->size()], &_data[dest]);
		_lock.unlock();
		source->_lock.unlock();
	}
	
	void removeEntity(size_t index)
	{
		_lock.lock();
		assert(_size > 0);
		assert(index < _size);

		// Deconstruct the components of the removed entity
		for (size_t c = 0; c < _archetype->components().size(); c++)
		{
			if (_archetype->components()[c]->size() != 0)
				_archetype->components()[c]->deconstruct(getComponentData(c, index));
		}

		//Copy last index to the one that we are removing
		if (index != _size - 1)
		{
			for (size_t i = 0; i < _archetype->components().size(); i++)
			{
				if (_archetype->components()[i]->size() != 0)
					std::copy(getComponentData(i, _size - 1), getComponentData(i, _size - 1), getComponentData(i, index));
			}
		}

		//Remove the last index
		--_size;
		_lock.unlock();
	}

	void clear()
	{
		_lock.lock();
		if (_archetype)
		{
			for (size_t c = 0; c < _archetype->components().size(); c++)
			{
				for (size_t i = 0; i < _size; i++)
				{
					//_archetype->components()[c]->deconstruct(getComponent(c, i));
				}
			}
		}
		_data.fill(0);
		_size = 0;
		_lock.unlock();
	}

	size_t size()
	{
		_lock.lock_shared();
		size_t size = _size;
		_lock.unlock_shared();
		return size;
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

	void lock()
	{
		_lock.lock();
	}
	void unlock()
	{
		_lock.unlock();
	}
	void lock_shared()
	{
		_lock.lock_shared();
	}
	void unlock_shared()
	{
		_lock.unlock_shared();
	}
};

typedef ChunkBase<16384> Chunk;



class ChunkPool
{
#ifdef TEST_BUILD
public:
#endif
	std::mutex _m;
	std::vector<std::unique_ptr<Chunk>> _unused;

public:
	~ChunkPool();
	friend void operator>>(ChunkPool& pool, std::unique_ptr<Chunk>& dest);
	friend void operator<<(ChunkPool& pool, std::unique_ptr<Chunk>& dest);
};