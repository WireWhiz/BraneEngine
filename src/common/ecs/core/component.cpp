#include "component.h"
#include <algorithm>
#include <cstring>

VirtualComponent::VirtualComponent(const VirtualComponent& source)
{
	_def = source._def;
	_data = new byte[_def->size()];
	for (auto& type : _def->types())
	{
		type->copy(&_data[type->offset()], &source._data[type->offset()]);
	}
}

VirtualComponent::VirtualComponent(VirtualComponent&& source)
{
	_def = source._def;
	_data = source._data;
	source._data = nullptr;
}

VirtualComponent::VirtualComponent(const ComponentAsset* definition)
{
	_def = definition;
	_data = new byte[_def->size()];
	for(auto& type : _def->types())
	{
		type->construct(&_data[type->offset()]);
	}
}

VirtualComponent::VirtualComponent(const ComponentAsset* definition, const byte* const data)
{
	_def = definition;
	_data = new byte[_def->size()];
	for (auto& type : _def->types())
	{
		type->copy(&_data[type->offset()], &data[type->offset()]);
	}
}

VirtualComponent::~VirtualComponent()
{
	for (auto& type : _def->types())
	{
		type->deconstruct(&_data[type->offset()]);
	}
	if(_data)
		delete[] _data;
}

VirtualComponent& VirtualComponent::operator=(const VirtualComponent& source)
{
	_def = source._def;
	_data = new byte[_def->size()];
	for (auto& type : _def->types())
	{
		type->copy(&_data[type->offset()], &source._data[type->offset()]);
	}
	return *this;
}

byte* VirtualComponent::data() const
{
	return _data;
}


const ComponentAsset* VirtualComponent::def() const
{
	return _def;
}



VirtualComponentVector::VirtualComponentVector(const ComponentAsset* definition)
{
	_def = definition;
}

VirtualComponentVector::VirtualComponentVector(const VirtualComponentVector& other)
{
	_def = other._def;
}

VirtualComponentVector::VirtualComponentVector(const ComponentAsset* const definition, size_t initalSize)
{
	_def = definition;
	reserve(initalSize * _def->size());
}

size_t VirtualComponentVector::structIndex(size_t index) const
{
	return index * _def->size();
}

const ComponentAsset* VirtualComponentVector::def() const
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

void VirtualComponentVector::remove(size_t index)
{
	if (_def->size() == 0)
		return;

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
	std::memcpy(&_data[structIndex(destIndex)], &source->_data[structIndex(sourceIndex)], _def->size());
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

VirtualComponentPtr::VirtualComponentPtr(const ComponentAsset* definition, byte* const data)
{
	_def = definition;
	_data = data;
}

const ComponentAsset* VirtualComponentPtr::def() const
{
	return _def;
}

byte* VirtualComponentPtr::data() const
{
	return _data;
}

