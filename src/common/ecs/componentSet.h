#pragma once
#include <set>
#include <cassert>
#include <cstddef>
#include <vector>
#include "assets/types/componentAsset.h"

// Class that always has components sorted
class ComponentSet
{
private:
	std::vector<ComponentID> _components;
public:
	ComponentSet() = default;
	ComponentSet(const std::vector<ComponentID>& components);
	void add(ComponentID component);
	void remove(ComponentID component);

	bool contains(ComponentID component) const;
	bool contains(const ComponentSet& subset) const;
	size_t index(ComponentID component) const;
	size_t size() const;

	ComponentID operator[](size_t index) const;
	bool operator==(const ComponentSet&) const;
	std::vector<ComponentID>::const_iterator begin() const;
	std::vector<ComponentID>::const_iterator end() const;

	void indices(const ComponentSet& subset, std::vector<size_t>& offsets) const;
};