#include "componentSet.h"

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
	if (subset.size() == 0)
		return false;
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

ComponentSet::ComponentSet(std::vector<const ComponentAsset*> components)
{
	for(auto& c : components)
		add(c);
}
