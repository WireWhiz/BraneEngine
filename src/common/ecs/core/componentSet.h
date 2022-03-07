#pragma once
#include <set>
#include <cassert>
#include <cstddef>
#include <vector>

class ComponentAsset;

// Class that always has components sorted
class ComponentSet
{
private:
	std::set<const ComponentAsset*> _components;
public:
	ComponentSet() = default;
	ComponentSet(std::vector<const ComponentAsset*> components);
	void add(const ComponentAsset* component);
	void remove(const  ComponentAsset* component);

	bool contains(const ComponentAsset* component) const;
	bool contains(const ComponentSet& subset) const;
	size_t index(const ComponentAsset* component) const;
	void indicies(const ComponentSet& subset, size_t* indices) const;

	size_t size() const;

	const ComponentAsset* operator[](size_t index) const;
	typename std::set<const ComponentAsset*>::const_iterator begin() const;
	typename std::set<const ComponentAsset*>::const_iterator end() const;
};