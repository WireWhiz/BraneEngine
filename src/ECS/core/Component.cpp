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
}

VirtualComponent::VirtualComponent(ComponentDefinition* definition, byte* data)
{
	_def = definition;
	_data = new byte[_def->size()];
	std::copy(data, data + _def->size() - 1, _data);
}

VirtualComponent::~VirtualComponent()
{
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



ComponentDefinition::ComponentDefinition()
{
	assert(false);
}

ComponentID ComponentDefinition::id() const
{
	return _id;
}

ComponentDefinition::ComponentDefinition(const ComponentDefinition& source)
{
	_numTypes = source._numTypes;
	_size = source._size;
	_initalized = source._initalized;
	_types = new VirtualType[_numTypes];
	std::copy(source._types, source._types + _numTypes, _types);
	_byteIndices = new size_t[_numTypes];
	std::copy(source._byteIndices, source._byteIndices + _numTypes, _byteIndices);
	_id = source._id;
}

ComponentDefinition::ComponentDefinition(size_t numTypes, ComponentID id)
{
	assert(numTypes > 0);
	_numTypes = numTypes;
	_size = 0;
	_id = id;
	_initalized = false;
	_byteIndices = new size_t[numTypes];
	_types = new VirtualType[numTypes];
	for (size_t i = 0; i < numTypes; i++)
	{
		_types[i] = virtualNone;
	}
}

ComponentDefinition::~ComponentDefinition()
{
	delete[] _types;
	delete[] _byteIndices;
}

void ComponentDefinition::setIndexType(size_t index, VirtualType type)
{
	assert(0 <= index && index < _numTypes);
	assert(type != virtualNone); // Can't set type to none
	_types[index] = type;
	
}

void ComponentDefinition::initalize()
{
	_initalized = true;
	for (size_t i = 0; i < _numTypes; i++)
	{
		assert(_types[i] != virtualNone); // Make sure all _types are initallized
		_byteIndices[i] = _size;
		_size += sizeofVirtual(_types[i]);
	}
}

void ComponentDefinition::initalize(const std::vector<NativeVarDef> vars, size_t size)
{
	assert(vars.size() == _numTypes);
	_initalized = true;
	_size = size;
	for (size_t i = 0; i < _numTypes; i++)
	{
		assert(vars[i].type != virtualNone); // Make sure all _types are initallized
		_types[i] = vars[i].type;
		_byteIndices[i] = vars[i].index;
	}
}

size_t ComponentDefinition::size() const
{
	assert(_initalized);
	return _size;
}

size_t ComponentDefinition::getByteIndex(size_t index) const
{
	assert(_initalized);
	assert(index >= 0 && index < _numTypes);
	return _byteIndices[index];
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

size_t VirtualComponentVector::byteIndex(size_t structIndex) const
{
	return structIndex * _def->size();
}

ComponentDefinition* VirtualComponentVector::def() const
{
	return _def;
}

size_t VirtualComponentVector::size() const
{
	return _data.size() / _def->size();
}

byte* VirtualComponentVector::getComponentData(size_t index) const
{
	assert(index >= 0 && index < _data.size() / _def->size());
	return (byte*)&_data[ byteIndex(index)];
}

VirtualComponentPtr VirtualComponentVector::getComponent(size_t index) const
{
	return VirtualComponentPtr(_def, (byte*)getComponentData(index));
}

void VirtualComponentVector::swapRemove(size_t index)
{
	assert(index >= 0 && index < _data.size() / _def->size());
	std::memcpy(&_data[byteIndex(index)], &_data[_data.size() -_def->size()], _def->size());
	_data.resize(_data.size() - _def->size());
}

void VirtualComponentVector::copy(const VirtualComponentVector* source, size_t sourceIndex, size_t destIndex)
{
	std::memcpy(&_data[byteIndex(destIndex)], &source->_data[sourceIndex], _def->size());
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
	_data.resize(_def->size() + _data.size());
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
