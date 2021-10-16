#include "Component.h"
#include <algorithm>
#include <cstring>

VirtualComponent::VirtualComponent(const VirtualComponent& source)
{
	_def = source._def;
	_data = new byte[_def->size()];
	std::copy(source._data, source._data + _def->size() - 1, _data);
}

VirtualComponent::VirtualComponent(ComponentDefinition* definition)
{
	_def = definition;
	_data = new byte[_def->size()];
	for(auto& type : _def->types())
	{
		type->construct(&_data[type->offset()]);
	}
}

VirtualComponent::VirtualComponent(ComponentDefinition* definition, byte* data)
{
	_def = definition;
	_data = new byte[_def->size()];
	std::copy(data, data + _def->size() - 1, _data);
}

VirtualComponent::~VirtualComponent()
{
	for (auto& type : _def->types())
	{
		type->deconstruct(&_data[type->offset()]);
	}
	delete[] _data;
}

ComponentDefinition* VirtualComponent::def() const
{
	return _def;
}

byte* VirtualComponent::data() const
{
	return _data;
}

ComponentID ComponentDefinition::id() const
{
	return _id;
}

ComponentDefinition::ComponentDefinition(const ComponentDefinition& source)
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

ComponentDefinition::ComponentDefinition(std::vector<std::shared_ptr<VirtualType>>& types, ComponentID id)
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

ComponentDefinition::~ComponentDefinition()
{
}

void ComponentDefinition::setSize(size_t size)
{
	_size = size;
}

size_t ComponentDefinition::size() const
{
	return _size;
}

size_t ComponentDefinition::getByteIndex(size_t index) const
{
	assert(index >= 0 && index < _types.size());
	return _types[index]->offset();
}

const std::vector<std::shared_ptr<VirtualType>>& ComponentDefinition::types()
{
	return _types;
}

VirtualComponentVector::VirtualComponentVector(ComponentDefinition* definition)
{
	_def = definition;
}

VirtualComponentVector::VirtualComponentVector(const VirtualComponentVector& other)
{
	_def = other._def;
}

VirtualComponentVector::VirtualComponentVector(ComponentDefinition* definition, size_t initalSize)
{
	_def = definition;
	reserve(initalSize * _def->size());
}

size_t VirtualComponentVector::structIndex(size_t index) const
{
	return index * _def->size();
}

ComponentDefinition* VirtualComponentVector::def() const
{
	return _def;
}

size_t VirtualComponentVector::size() const
{
	if (_def->size() == 0)
		return 0;
	return _data.size() / _def->size();
}

byte* VirtualComponentVector::getComponentData(size_t index) const
{
	if (_def->size() == 0)
		return nullptr;
	assert(index >= 0 && index * _def->size() < _data.size());
	return (byte*)&_data[ structIndex(index)];
}

VirtualComponentPtr VirtualComponentVector::getComponent(size_t index) const
{
	return VirtualComponentPtr(_def, (byte*)getComponentData(index));
}

void VirtualComponentVector::swapRemove(size_t index)
{
	assert(index >= 0 && index < _data.size() / _def->size());
	for (auto& type : _def->types())
	{
		type->deconstruct(&_data[structIndex(index) + type->offset()]);
	}
	std::memcpy(&_data[structIndex(index)], &_data[_data.size() -_def->size()], _def->size());
	_data.resize(_data.size() - _def->size());
	
}

void VirtualComponentVector::copy(const VirtualComponentVector* source, size_t sourceIndex, size_t destIndex)
{
	std::memcpy(&_data[structIndex(destIndex)], &source->_data[sourceIndex], _def->size());
}

void VirtualComponentVector::pushBack(VirtualComponent& virtualStruct)
{
	assert(virtualStruct.def() == _def);
	byte* inData = virtualStruct.data();
	for (size_t i = 0; i < _def->size(); i++)
	{
		_data.push_back(*(inData + i));
	}
}

void VirtualComponentVector::pushBack(VirtualComponentPtr& virtualComponentPtr)
{
	assert(virtualComponentPtr.def() == _def);
	byte* inData = virtualComponentPtr.data();
	for (size_t i = 0; i < _def->size(); i++)
	{
		_data.push_back(*(inData + i));
	}
}

void VirtualComponentVector::pushEmpty()
{
	size_t index = _data.size();
	_data.resize(_def->size() + index);
	for (auto& type : _def->types())
	{
		type->construct(&_data[index + type->offset()]);
	}
}

void VirtualComponentVector::reserve(size_t size)
{
	_data.reserve(size * _def->size());
}

VirtualComponentPtr::VirtualComponentPtr(ComponentDefinition* definition, byte* data)
{
	_def = definition;
	_data = data;
}

ComponentDefinition* VirtualComponentPtr::def() const
{
	return _def;
}

byte* VirtualComponentPtr::data() const
{
	return _data;
}

void VirtualComponentChunk::setDef(std::vector<ComponentDefinition*>& _def)
{

}

VirtualComponentChunk::VirtualComponentChunk(size_t size) : _size(size)
{
	_maxCapacity = 0;
}
