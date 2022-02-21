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
	_header.id = id;
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
	_header.id = id;
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

void ComponentAsset::serializeComponent(OSerializedData& sdata, byte* component) const
{
	for (int i = 0; i < _types.size(); ++i)
	{
		_types[i]->serialize(sdata, component + _types[i]->offset());
	}
}

void ComponentAsset::deserializeComponent(ISerializedData& sdata, byte* component) const
{
	for (int i = 0; i < _types.size(); ++i)
	{
		 _types[i]->deserialize(sdata, component + _types[i]->offset());
	}
}

void ComponentAsset::serialize(OSerializedData& sdata)
{
	Asset::serialize(sdata);
	sdata << (uint32_t)_types.size();
	for(auto& type : _types)
	{
		sdata << (uint16_t)type->getType();
	}
}

void ComponentAsset::deserialize(ISerializedData& sdata)
{
	Asset::deserialize(sdata);
	uint32_t size;
	sdata.readSafeArraySize(size);
	std::vector<std::unique_ptr<VirtualType>> types;
	types.reserve(size); // Creating a local vector instead of using the classes one, so it doesn't get overwritten when we call the constructor. ID gets copied, so we don't need to worry about that.
	for(uint32_t i = 0; i < size; i++)
	{
		uint16_t type;
		sdata >> type;
		types.push_back(std::unique_ptr<VirtualType>(VirtualType::constructTypeOf((VirtualType::Type)type)));
	}
	new(this) ComponentAsset(types, id());
}

ComponentAsset::ComponentAsset(ISerializedData& sData)
{
	deserialize(sData);
}
