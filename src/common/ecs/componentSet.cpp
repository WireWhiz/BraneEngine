#include <vector>
#include <algorithm>
#include "componentSet.h"

void ComponentSet::add(ComponentID id)
{
	auto i = _components.begin();
	for(; i != _components.end() && *i < id; i++);
	_components.insert(i, id);
}

void ComponentSet::remove(ComponentID id)
{
	auto i = _components.begin();
	for(; i != _components.end() && *i != id; i++);
	assert(i != _components.end());
	_components.erase(i);
}

bool ComponentSet::contains(ComponentID id) const
{
	for(auto c : _components)
		if(c == id) return true;
	return false;
}

bool ComponentSet::contains(const ComponentSet& subset) const
{
	if (subset.size() == 0)
		return false;
	auto si = subset._components.begin();
	auto i = _components.begin();
	while(si != subset._components.end())
	{
		while(*si != *i){
			i++;
			if(i == _components.end())
				return false;
		}
		si++;
	}
	return true;
}

size_t ComponentSet::index(ComponentID id) const
{
	for(uint16_t i = 0; i < _components.size(); i++)
		if(_components[i] == id)
			return i;
	assert(false && "ID not found");
	return -1;
}

size_t ComponentSet::size() const
{
	return _components.size();
}

ComponentID ComponentSet::operator[](size_t index) const
{
	return _components[index];
}

typename std::vector<ComponentID>::const_iterator ComponentSet::begin() const
{
	return _components.begin();
}

typename  std::vector<ComponentID>::const_iterator ComponentSet::end() const
{
	return _components.end();
}

ComponentSet::ComponentSet(const std::vector<ComponentID>& components)
{
	_components = components;
	std::sort(_components.begin(), _components.end());
}

void ComponentSet::indices(const ComponentSet& subset, std::vector<size_t>& offsets) const
{
	size_t si = 0;
	size_t i = 0;
	while(si != subset._components.size())
	{
		while(subset._components[si] != _components[i]){
			i++;
			assert(i != _components.size());
		}
		offsets[si] = i;
		si++;
	}
}
