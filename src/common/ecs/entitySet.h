//
// Created by eli on 7/16/2022.
//

#ifndef BRANEENGINE_ENTITYSET_H
#define BRANEENGINE_ENTITYSET_H

#include <cstdint>
#include <vector>/*
#include "archetype.h"
#include "system.h"*/
#include "chunk.h"

using ComponentID = uint16_t;
class SystemContext;
class ChunkComponentView;
class Archetype;

typedef uint8_t ComponentFilterFlags;
enum ComponentFilterFlags_
{
	ComponentFilterFlags_None    = 0,
	ComponentFilterFlags_Const   = 1 << 0,
	ComponentFilterFlags_Changed = 1 << 1,
	ComponentFilterFlags_Exclude = 1 << 2
};

class ComponentFilter
{
	SystemContext* _system;
public:
	struct Component
	{
		ComponentID id;
		ComponentFilterFlags flags;
	};

private:
	std::vector<Component> _components;
public:
	ComponentFilter(SystemContext* system);
	void addComponent(ComponentID id, ComponentFilterFlags flags = ComponentFilterFlags_None);
	const std::vector<ComponentFilter::Component>& components() const;
	SystemContext* system() const;

	bool checkArchetype(Archetype* arch) const;
	bool checkChunk(Chunk* chunk) const;
};

class EntitySet
{
	ComponentFilter _filter;
	std::vector<ChunkComponentView*> _components;
public:
	EntitySet(std::vector<ChunkComponentView*> components, ComponentFilter filter);
	void forEachNative(const std::function<void(byte** components)>& f);
};


#endif //BRANEENGINE_ENTITYSET_H
