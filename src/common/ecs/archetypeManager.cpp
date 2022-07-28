//
// Created by eli on 7/17/2022.
//

#include "archetypeManager.h"
#include "componentManager.h"

ArchetypeManager::ArchetypeManager(ComponentManager& componentManager) : _componentManager(componentManager)
{
	_chunkAllocator = std::make_shared<ChunkPool>();
}

Archetype* ArchetypeManager::makeArchetype(const ComponentSet& components)
{
	ASSERT_MAIN_THREAD();
	assert(components.size() > 0);
	size_t numComps = components.size();

	// We want to keep archetypes of the same size in groups
	// This means we keep each size in a different vector

	// Make sure we have enough vectors
	if (numComps > _archetypes.size())
		_archetypes.resize(numComps);

	std::vector<const ComponentDescription*> descriptions;
	descriptions.reserve(components.size());
	for(auto id : components)
		descriptions.push_back(_componentManager.getComponentDef(id));

	size_t newIndex = _archetypes[numComps - 1].size();
	_archetypes[numComps - 1].push_back(std::make_unique<Archetype>(descriptions, _chunkAllocator));

	Archetype* newArch = _archetypes[numComps - 1][newIndex].get();



	{//Scope for bool array
		// find edges to other archetypes lower than this one
		if (numComps > 1)
		{
			ComponentID connectingComponent;
			for (size_t i = 0; i < _archetypes[numComps - 2].size(); i++)
			{
				if (_archetypes[numComps - 2][i]->isChildOf(newArch, connectingComponent))
				{
					Archetype* otherArch = _archetypes[numComps - 2][i].get();
					otherArch->addAddEdge(connectingComponent, newArch);
					newArch->addRemoveEdge(connectingComponent, otherArch);
				}
			}
		}

		// find edges to other archetypes higher than this one
		if (_archetypes.size() > numComps)
		{
			for (size_t i = 0; i < _archetypes[numComps].size(); i++)
			{
				ComponentID connectingComponent;
				if (newArch->isChildOf(_archetypes[numComps][i].get(), connectingComponent))
				{
					Archetype* otherArch = _archetypes[numComps][i].get();
					newArch->addAddEdge(connectingComponent, otherArch);
					otherArch->addRemoveEdge(connectingComponent, newArch);
				}
			}
		}
	}
	return newArch;
}

Archetype* ArchetypeManager::getArchetype(const ComponentSet& components)
{
	ASSERT_MAIN_THREAD();
	size_t numComps = components.size();
	assert(numComps > 0);
	if (numComps > _archetypes.size())
	{
		return makeArchetype(components);
	}
	auto& archetypes = _archetypes[numComps - 1];

	for (size_t a = 0; a < archetypes.size(); a++)
	{
		assert(archetypes[a]->components().size() == components.size());
		if (archetypes[a]->hasComponents(components))
		{
			return archetypes[a].get();
		}
	}
	return makeArchetype(components);
}

void ArchetypeManager::clear()
{
	_archetypes.resize(0);
}

EntitySet ArchetypeManager::getEntities(ComponentFilter filter)
{
	assert(filter.components().size() > 0);
	std::vector<ChunkComponentView*> componentViews;
	for (size_t a = filter.components().size() - 1; a < _archetypes.size(); ++a)
	{
		auto& archetypes = _archetypes[a];
		for(auto& arch : archetypes)
		{
			if(!filter.checkArchetype(arch.get()))
				continue;
			for(auto& chunk : arch->chunks())
			{
				if(!filter.checkChunk(chunk.get()))
					continue;
				for(auto& c : filter.components())
				{
					if(!(c.flags & ComponentFilterFlags_Exclude))
						componentViews.push_back(&chunk->getComponent(c.id));
				}
			}
		}
	}

	return {std::move(componentViews), std::move(filter)};
}
