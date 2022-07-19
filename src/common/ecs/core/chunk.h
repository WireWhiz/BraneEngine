#pragma once

#include <array>
#include <vector>
#include "virtualType.h"
#include "component.h"

class ChunkComponentView
{
	const ComponentDescription* _description = nullptr;
	byte* _data = nullptr;
	size_t _maxSize = 0;
	size_t _size = 0;
	byte* dataIndex(size_t index) const;
public:
	uint32_t version;
	ChunkComponentView() = default;
	ChunkComponentView(byte* data, size_t maxSize, const ComponentDescription* def);
	~ChunkComponentView();
	VirtualComponentView operator[](size_t index) const;
	void createComponent();
	void erase(size_t index);
	void setComponent(size_t index, VirtualComponentView component);
	void setComponent(size_t index, VirtualComponent&& component);
	byte* getComponentData(size_t index);

	size_t size() const;
	uint32_t compID() const;
	const ComponentDescription* def();
};

template<size_t N>
class ChunkBase
{
#ifdef TEST_BUILD
public:
#endif
	size_t _size;
    size_t _maxCapacity;
    std::array<byte, N> _data;
    std::vector<ChunkComponentView> _components;
public:
	ChunkBase()
	{
		_size = 0;
	}
	ChunkBase(const ChunkBase&) = delete;
	~ChunkBase()
	{
		clear();
	}
	void setComponents(std::vector<const ComponentDescription*> components)
	{
		clear();
        _components.reserve(components.size());
        byte* componentDataStart = _data.data();
        size_t entitySize = 0;
        for (auto& c : components)
            entitySize += c->size();
        _maxCapacity = N / entitySize;

        for (auto& c : components)
        {
            _components.emplace_back(componentDataStart, _maxCapacity, c);
            componentDataStart += c->size() * _maxCapacity;
        }
	}

	const std::vector<ChunkComponentView>& components()
	{
		return _components;
	}

	bool hasComponent(uint32_t id)
	{
		for(auto& cv : _components)
			if(cv.compID() == id)
				return true;
		return false;
	}

	ChunkComponentView& getComponent(uint32_t id)
	{
		for(auto& cv : _components)
			if(cv.compID() == id)
				return cv;
		throw std::runtime_error("Tried to access component not contained by chunk");
	}

	bool tryGetComponent(uint32_t id, ChunkComponentView*& view)
	{
		for(auto& cv : _components)
			if(cv.compID() == id)
			{
				view = &cv;
				return true;
			}
		return false;
	}

	size_t createEntity()
	{
		assert(_size < _maxCapacity);
		for(auto& c : _components)
			c.createComponent();
		return _size++;
	}

	void moveEntity(ChunkBase* dest, size_t sIndex, size_t dIndex)
	{
		assert(sIndex < _size);
		assert(dIndex < dest->_size);
		for(auto& c : _components)
		{
			ChunkComponentView* oc = nullptr;
			if(dest->tryGetComponent(c.compID(), oc))
			{
				c.def()->move(c[sIndex].data(), (*oc)[dIndex].data());
				oc->version = std::max(oc->version, c.version);
			}
		}
	}
	
	void removeEntity(size_t index)
	{
		assert(index < _size);
		for(auto& c : _components)
			c.erase(index);
		--_size;
	}

	void clear()
	{
		_components.resize(0);
		_size = 0;
        _maxCapacity = 0;
	}

	size_t size()
	{
		return _size;
	}

	size_t maxCapacity()
	{
		return _maxCapacity;
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

template <size_t>
class ChunkBase;
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