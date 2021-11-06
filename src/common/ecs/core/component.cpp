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
	size_t insertionIndex = 0;
	for (size_t i = 0; i < _components.size(); i++)
	{
		assert(component != _components[i]); // Can't add the same item twice
		if ((size_t)component > (size_t)_components[i])
		{
			insertionIndex += 1;
		}
		else
			break;
	}
	_components.insert(_components.begin() + insertionIndex, component);
}

void ComponentSet::remove(const ComponentAsset* component)
{
	assert(component != nullptr);
	for (size_t i = 0; i < _components.size(); i++)
	{
		if (_components[i] == component)
		{
			_components.erase(_components.begin() + i);
			break;
		}
	}
}

const std::vector<const ComponentAsset*>& ComponentSet::components() const
{
	return _components;
}

bool ComponentSet::contains(const ComponentAsset* component) const
{
	size_t start = 0;
	size_t end = _components.size() - 1;
	while(true)
	{
		size_t middle = (start + end) / 2;
		if (_components[middle] == component)
			return true;

		if ((size_t)_components[middle] < (size_t)component)
			start = middle + 1;
		else if ((size_t)_components[middle] > (size_t)component)
			end = middle - 1;

		if (end < start || end > _components.size())
			return false;
		
	}
	return false;
}

bool ComponentSet::contains(const ComponentSet& subset) const
{
	size_t count = 0;
	for (size_t i = 0; i < _components.size(); i++)
	{
		if ((size_t)_components[i] > (size_t)subset._components[count])
			return false;
		if (_components[i] == subset._components[count])
		{
			if (++count == subset._components.size())
				return true;
		}
			
	}
	return false;
}

size_t ComponentSet::index(const ComponentAsset* component) const
{
	size_t start = 0;
	size_t end = _components.size() - 1;
	while (true)
	{
		size_t middle = (start + end) / 2;
		if (_components[middle] == component)
			return middle;

		if ((size_t)_components[middle] < (size_t)component)
			start = middle + 1;
		else if ((size_t)_components[middle] > (size_t)component)
			end = middle - 1;

		if (end < start || end > _components.size())
			return nullindex;
	}
}

void ComponentSet::indicies(const ComponentSet& subset, size_t* indices) const
{
	size_t count = 0;
	for (size_t i = 0; i < _components.size(); i++)
	{
		if ((size_t)_components[i] > (size_t)subset._components[count])
			assert(false && "values not found");
		if (_components[i] == subset._components[count])
		{
			indices[count] = i;
			if (++count == subset._components.size())
				return;
		}

	}
}

size_t ComponentSet::size() const
{
	return _components.size();
}

const ComponentAsset* ComponentSet::operator[](size_t index) const
{
	return _components[index];
}

typename std::vector<const ComponentAsset*>::const_iterator ComponentSet::begin() const
{
	return _components.begin();
}

typename  std::vector<const ComponentAsset*>::const_iterator ComponentSet::end() const
{
	return _components.end();
}
