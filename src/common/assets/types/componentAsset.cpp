#include "componentAsset.h"



ComponentAsset::ComponentAsset(const ComponentAsset& source)
{
	_size = source._size;
	_types.resize(source._types.size());
	for (size_t i = 0; i < source._types.size(); i++)
	{
		_types[i] = source._types[i];
	}
	_types = source._types;
	_id = source._id;
}

ComponentAsset::ComponentAsset(const std::vector<std::shared_ptr<VirtualType>>& types, AssetID id)
{
	_size = 0;
	_id = id;
	if (types.size() != 0)
	{

		_types.resize(types.size());
		for (size_t i = 0; i < types.size(); i++)
		{
			_types[i] = types[i];
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

const std::vector<std::shared_ptr<VirtualType>>& ComponentAsset::types() const
{
	return _types;
}