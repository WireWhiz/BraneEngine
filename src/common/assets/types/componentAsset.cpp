#include "componentAsset.h"

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
		
		if (_types[0]->offset() != 0 || _types.size() > 1 && _types[1]->offset() != 0)
			return; // This means that they have already been set
		for (size_t i = 0; i < _types.size(); i++)
		{
			_types[i]->setOffset(_size);
			_size += _types[i]->size();
		}
	}


}

ComponentAsset::~ComponentAsset()
{
}

void ComponentAsset::setSize(size_t size)
{
	_size = size;
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