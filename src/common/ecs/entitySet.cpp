//
// Created by eli on 7/16/2022.
//

#include "entitySet.h"
#include "system.h"
#include "archetype.h"

void ComponentFilter::addComponent(ComponentID id, ComponentFilterFlags flags)
{
	_components.push_back({id, flags});
}

const std::vector<ComponentFilter::Component>& ComponentFilter::components() const
{
	return _components;
}

ComponentFilter::ComponentFilter(SystemContext* system)
{
	_system = system;
}

SystemContext* ComponentFilter::system() const
{
	return _system;
}

bool ComponentFilter::checkChunk(Chunk* chunk) const
{
	for (auto& comp : _components)
	{
        if(comp.flags & ComponentFilterFlags_Exclude)
            continue;
		assert(chunk->hasComponent(comp.id));
		auto& view = chunk->getComponent(comp.id);
		if(comp.flags & ComponentFilterFlags_Changed && (view.version <= _system->lastVersion))
			return false;
	}
	return true;
}

bool ComponentFilter::checkArchetype(Archetype* arch) const
{
	for(auto& c : _components)
	{
		bool hasComponent = arch->hasComponent(c.id);
        bool exclude = c.flags & ComponentFilterFlags_Exclude;
		if(hasComponent == exclude)
			return false;
	}
	return true;
}

EntitySet::EntitySet(std::vector<ChunkComponentView*> components, ComponentFilter filter) : _components(std::move(components)), _filter(std::move(filter))
{
}

void EntitySet::forEachNative(const std::function<void(byte** components)>& f)
{
	size_t numComps = 0;
    for(auto& c : _filter.components())
        if(!(c.flags & ComponentFilterFlags_Exclude))
            ++numComps;

	byte** data = (byte**)STACK_ALLOCATE(sizeof(const byte*) * numComps);

	for (int i = 0; i < _components.size() / numComps; ++i)
	{
		size_t setIndex = i * numComps;
		for(int e = 0; e < _components[setIndex]->size(); ++e)
		{
			for (int c = 0; c < numComps; ++c)
			{
				data[c] = (*_components[setIndex + c])[e].data();
			}
			f(data);
		}
		for (int c = 0; c < numComps; ++c)
		{
			if(!(_filter.components()[c].flags & ComponentFilterFlags_Const))
				_components[setIndex + c]->version = _filter.system()->version;
		}
	}
}
