#pragma once
#include <set>
#include <cassert>
#include <cstddef>
#include <vector>
#include "assets/types/componentAsset.h"
#include "robin_hood.h"

// Class that always has components sorted

using ComponentID = uint32_t;
class ComponentSet
{
private:
    robin_hood::unordered_flat_set<ComponentID> _components;
public:
    ComponentSet() = default;
    ComponentSet(const std::vector<ComponentID>& components);
    void add(ComponentID component);
    void remove(ComponentID component);

    bool contains(ComponentID component) const;
    bool contains(const ComponentSet& subset) const;
    size_t size() const;

    bool operator==(const ComponentSet&) const;
    robin_hood::unordered_flat_set<ComponentID>::const_iterator begin() const;
    robin_hood::unordered_flat_set<ComponentID>::const_iterator end() const;
};