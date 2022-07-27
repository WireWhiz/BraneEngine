#pragma once
#include <cstdint>
#include <memory>
#include <functional>
#include <unordered_map>
#include <common/utility/stackAllocate.h>
#include <utility/sharedRecursiveMutex.h>
#include <utility/threadPool.h>
#include <unordered_set>
#include <list>

#include "component.h"
#include "componentSet.h"
#include "chunk.h"

class Archetype;
class EntitySet;
class ComponentFilter;

struct ArchetypeEdge
{
	ComponentID component;
	Archetype* archetype;
	ArchetypeEdge(ComponentID component, Archetype* archetype);
};

class Archetype
{
#ifdef TEST_BUILD
public:
#endif
	size_t _size = 0;
	size_t _entitySize;

	//Eventually move these to separate node class
	std::vector<std::shared_ptr<ArchetypeEdge>> _addEdges;
	std::vector<std::shared_ptr<ArchetypeEdge>> _removeEdges;
	//

	ComponentSet _components;
	std::vector<const ComponentDescription*> _componentDescriptions;
	std::vector<std::unique_ptr<Chunk>> _chunks;
	std::shared_ptr<ChunkPool> _chunkAllocator;

	size_t chunkIndex(size_t entity) const;

	Chunk* getChunk(size_t entity) const;
public:
	Archetype(const std::vector<const ComponentDescription*>& components, std::shared_ptr<ChunkPool>& _chunkAllocator);
	~Archetype();
	bool hasComponent(ComponentID component) const;
	bool hasComponents(const ComponentSet& comps) const;
	bool hasComponents(const ComponentFilter& filter) const;
	VirtualComponentView getComponent(size_t entity, ComponentID component) const;
    void setComponentVersion(size_t entity, ComponentID component, uint32_t version);
	void setComponent(size_t entity, VirtualComponent&& component);
	void setComponent(size_t entity, VirtualComponentView component);
	bool isChildOf(const Archetype* parent, ComponentID& connectingComponent) const;
	const ComponentSet& components() const;
	const std::vector<const ComponentDescription*>& componentDescriptions();
	std::shared_ptr<ArchetypeEdge> getAddEdge(ComponentID component);
	std::shared_ptr<ArchetypeEdge> getRemoveEdge(ComponentID component);
	void addAddEdge(ComponentID component, Archetype* archetype);
	void addRemoveEdge(ComponentID component, Archetype* archetype);
	const std::vector<std::unique_ptr<Chunk>>& chunks() const;
	size_t size() const;
	size_t createEntity();
	size_t moveEntity(size_t index, Archetype* dest);
	void removeEntity(size_t index);
	size_t entitySize() const;

	friend class ArchetypeView;
};


