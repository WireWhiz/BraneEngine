#pragma once
#include <array>
#include <vector>
#include "virtualType.h"
#include <algorithm>
#include <functional>
#include <utility/shared_recursive_mutex.h>

#include "archetype.h"
class Archetype;

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
		_lock.lock();
		clear();
		_archetype = archetype;

		if (archetype)
		{
			_componentIndices = std::vector<size_t>(_archetype->components().size());
			for (size_t i = 0; i < _archetype->components().size(); i++)
			{
				if (i != 0)
					_componentIndices[i] = _componentIndices[i - 1] + _archetype->componentDescriptions()[i - 1]->size() * maxCapacity();
			}
		}
		_lock.unlock();
	}

	byte* getComponentData(const ComponentDescription* component, size_t entity)
	{

		return &_data[_componentIndices[_archetype->components().index(component->id)] + entity * component->size()];
	}
	
	byte* getComponentData(size_t component, size_t entity)
	{

		return &_data[_componentIndices[component] + entity * _archetype->componentDescriptions()[component]->size()];
	}

	VirtualComponent getComponent(const ComponentDescription* component, size_t entity)
	{
		_lock.lock_shared();
		VirtualComponent o = VirtualComponent(component, getComponentData(component, entity));
		_lock.unlock_shared();
		return o;
	}

	void setComponent(const VirtualComponent& component, size_t entity)
	{
		_lock.lock();
		component.description()->copy(component.data(),getComponentData(_archetype->components().index(component.description()->id), entity));
		_lock.unlock();
	}

	void setComponent(const VirtualComponentView& component, size_t entity)
	{
		_lock.lock();
		component.description()->copy(component.data(), getComponentData(_archetype->components().index(component.description()->id), entity));
		_lock.unlock();
	}

	size_t createEntity()
	{
		_lock.lock();
		assert(_size < maxCapacity());
		size_t index = _size;
		for (size_t c = 0; c < _archetype->componentDescriptions().size(); c++)
		{
			if(_archetype->componentDescriptions()[c]->size() != 0)
				_archetype->componentDescriptions()[c]->construct(getComponentData(c, index));
		}
		++_size;
		_lock.unlock();
		return index;
	}

	void copyComponenet(ChunkBase* source, size_t sIndex, size_t dIndex, const ComponentDescription* component)
	{
		_lock.lock();
		source->_lock.lock();
		assert(source->_archetype->components().contains(component->id));
		assert(_archetype->components().contains(component->id));
		assert(sIndex < source->size());
		assert(dIndex < size());

		size_t sourceCompIndex = source->_archetype->components().index(component->id);
		size_t destCompIndex = _archetype->components().index(component->id);

		assert(sIndex + source->_componentIndices[sourceCompIndex] + component->size() < N);
		assert(dIndex + _componentIndices[destCompIndex] + component->size() < N);

		size_t src = sIndex * component->size() + source->_componentIndices[sourceCompIndex];
		size_t dest = dIndex * component->size() + _componentIndices[destCompIndex];

		component->copy(&source->_data[src], &_data[dest]);
		source->_lock.unlock();
		_lock.unlock();
	}
	
	void removeEntity(size_t index)
	{
		_lock.lock();
		assert(_size > 0);
		assert(index < _size);

		// Deconstruct the components of the removed entity
		for (size_t c = 0; c < _archetype->components().size(); c++)
		{
			if (_archetype->componentDescriptions()[c]->size() != 0)
				_archetype->componentDescriptions()[c]->deconstruct(getComponentData(c, index));
		}

		//Copy last index to the one that we are removing
		if (index != _size - 1)
		{
			for (size_t i = 0; i < _archetype->components().size(); i++)
			{
				if (_archetype->componentDescriptions()[i]->size() != 0)
					_archetype->componentDescriptions()[i]->move(getComponentData(i, _size - 1), getComponentData(i, index));
			}
		}

		//Remove the last index
		--_size;
		_lock.unlock();
	}

	void clear()
	{
		_lock.lock();
		if (_archetype && _size > 0)
		{
			for (auto c : _archetype->componentDescriptions())
			{
				for (size_t i = 0; i < _size; i++)
				{

					c->deconstruct(getComponentData(c, i));
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
	friend void operator>>(ChunkPool& pool, std::unique_ptr<Chunk>& dest);
	friend void operator<<(ChunkPool& pool, std::unique_ptr<Chunk>& src);
};