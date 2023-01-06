#pragma once
#include "assets/types/componentAsset.h"
#include "robin_hood.h"
#include <cassert>
#include <cstddef>
#include <set>
#include <vector>

// Class that always has components sorted
class ComponentSet {
private:
  robin_hood::unordered_flat_set<ComponentID> _components;

public:
  ComponentSet() = default;
  ComponentSet(const std::vector<ComponentID> &components);
  void add(ComponentID component);
  void remove(ComponentID component);

  bool contains(ComponentID component) const;
  bool contains(const ComponentSet &subset) const;
  size_t size() const;

  bool operator==(const ComponentSet &) const;
  robin_hood::unordered_flat_set<ComponentID>::const_iterator begin() const;
  robin_hood::unordered_flat_set<ComponentID>::const_iterator end() const;
};