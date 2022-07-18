//
// Created by eli on 7/17/2022.
//

#ifndef BRANEENGINE_ARCHETYPEMANAGER_H
#define BRANEENGINE_ARCHETYPEMANAGER_H

#include "archetype.h"
#include "entitySet.h"

class ComponentManager;
class ArchetypeManager
{
#ifdef TEST_BUILD
	public:
#endif

	std::shared_ptr<ChunkPool> _chunkAllocator;
	// Index 1: number of components, Index 2: archetype
	std::vector<std::vector<std::unique_ptr<Archetype>>> _archetypes;
	ComponentManager& _componentManager;
public:
	ArchetypeManager(ComponentManager& componentManager);
	Archetype* getArchetype(const ComponentSet& components);
	Archetype* makeArchetype(const ComponentSet& components);

	EntitySet getEntities(ComponentFilter filter);

	void removeEmpty();
	void clear();
};


#endif //BRANEENGINE_ARCHETYPEMANAGER_H
