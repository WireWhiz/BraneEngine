#include "componentAsset.h"

#ifdef _64BIT
#define WORD_SIZE 8
#endif
#ifdef _32BIT
#define WORD_SIZE 4
#endif

ComponentAsset::ComponentAsset(std::vector<std::unique_ptr<VirtualType>>& types, AssetID id)
{
	_size = 0;
	_id = id;
	if (types.size() != 0)
	{

		_types.resize(types.size());
		for (size_t i = 0; i < _types.size(); i++)
		{
			_types[i] = std::move(types[i]);
		}
		for (size_t i = 0; i < _types.size(); i++)
		{
			size_t woffset = _size % WORD_SIZE;
			if (woffset != 0 && WORD_SIZE - woffset < _types[i]->size())
			{
				_size += WORD_SIZE - woffset;
			}
			_types[i]->setOffset(_size);
			_size += _types[i]->size();

		}
	}


}

ComponentAsset::ComponentAsset(std::vector<VirtualType*>& types, AssetID id, size_t size)
{
	_size = size;
	_id = id;
	if (types.size() != 0)
	{
		_types.resize(types.size());
		for (size_t i = 0; i < _types.size(); i++)
		{
			_types[i] = std::unique_ptr<VirtualType>(types[i]);
		}
	}
}

ComponentAsset::~ComponentAsset()
{
}

size_t ComponentAsset::size() const
{
	return _size;
}

size_t ComponentAsset::getByteIndex(size_t index) const
{
	assert(index >= 0 && index < _types.size());
	return _types[index]->offset();
}

const AssetID& ComponentAsset::id() const
{
	return _id;
}

const std::vector<std::unique_ptr<VirtualType>>& ComponentAsset::types() const
{
	return _types;
}

void ComponentAsset::construct(byte* component) const
{
	for (size_t i = 0; i < _types.size(); i++)
	{
		_types[i]->construct(component + _types[i]->offset());
	}
}

void ComponentAsset::deconstruct(byte* component) const
{
	for (size_t i = 0; i < _types.size(); i++)
	{
		_types[i]->deconstruct(component + _types[i]->offset());
	}
}


void ComponentAsset::copy(byte* dest, byte* source) const
{
	for (size_t i = 0; i < _types.size(); i++)
	{
		_types[i]->copy(dest + _types[i]->offset(), source +_types[i]->offset());
	}
}

void ComponentAsset::move(byte* dest, byte* source) const
{
	for (size_t i = 0; i < _types.size(); i++)
	{
		_types[i]->move(dest + _types[i]->offset(), source + _types[i]->offset());
	}
}