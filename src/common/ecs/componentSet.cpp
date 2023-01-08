#include "componentSet.h"
#include <algorithm>
#include <vector>

void ComponentSet::add(ComponentID id) { _components.insert(id); }

void ComponentSet::remove(ComponentID id)
{
    assert(_components.contains(id));
    _components.erase(id);
}

bool ComponentSet::contains(ComponentID id) const { return _components.contains(id); }

bool ComponentSet::contains(const ComponentSet& subset) const
{
    if(subset.size() == 0)
        return false;
    for(auto c : subset)
        if(!_components.contains(c))
            return false;
    return true;
}

size_t ComponentSet::size() const { return _components.size(); }

typename robin_hood::unordered_flat_set<ComponentID>::const_iterator ComponentSet::begin() const
{
    return _components.begin();
}

typename robin_hood::unordered_flat_set<ComponentID>::const_iterator ComponentSet::end() const
{
    return _components.end();
}

ComponentSet::ComponentSet(const std::vector<ComponentID>& components)
{
    for(auto c : components)
        _components.insert(c);
}

bool ComponentSet::operator==(const ComponentSet& o) const
{
    if(o.size() != size())
        return false;
    for(auto c : _components)
        if(!o.contains(c))
            return false;
    return true;
}
