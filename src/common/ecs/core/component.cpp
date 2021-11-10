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

void ComponentSet::add(const ComponentAsset* component)
{
	assert(component != nullptr);
	_components.insert(component);
}

void ComponentSet::remove(const ComponentAsset* component)
{
	assert(component != nullptr);
	_components.erase(component);
}

bool ComponentSet::contains(const ComponentAsset* component) const
{
	return _components.count(component);
}

bool ComponentSet::contains(const ComponentSet& subset) const
{
	size_t count = 0;
	for (auto& component : subset)
	{
		if (!_components.count(component))
			return false;
	}
	return true;
}

size_t ComponentSet::index(const ComponentAsset* component) const
{
	assert(_components.count(component));
	return std::distance(_components.begin(), _components.find(component));
}

void ComponentSet::indicies(const ComponentSet& subset, size_t* indices) const
{
	size_t i = 0;
	for (auto c : subset)
	{
		assert(contains(c));
		indices[i++] = index(c);
	}
}

size_t ComponentSet::size() const
{
	return _components.size();
}

const ComponentAsset* ComponentSet::operator[](size_t index) const
{
	auto itter = _components.begin();
	std::advance(itter, index);
	return *itter;
}

typename std::set<const ComponentAsset*>::const_iterator ComponentSet::begin() const
{
	return _components.begin();
}

typename  std::set<const ComponentAsset*>::const_iterator ComponentSet::end() const
{
	return _components.end();
}
