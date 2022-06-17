#pragma once
#include <cstdint>
#include "component.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <common/utility/stackAllocate.h>
#include <utility/shared_recursive_mutex.h>
#include <utility/threadPool.h>
#include <unordered_set>
#include <list>
#include "componentSet.h"

class Archetype;

struct ArchetypeEdge
{
	ComponentID component;
	Archetype* archetype;
	ArchetypeEdge(ComponentID component, Archetype* archetype);
};

template <size_t N> class ChunkBase;
typedef ChunkBase<16384> Chunk;
class ChunkPool;

class ArchetypeView
{
	Archetype* _arch = nullptr;
	std::vector<size_t> componentSizes;
	std::vector<size_t> componentOffsets;
	std::vector<const ComponentDescription*> descriptions;
	ArchetypeView(Archetype& archetype, const ComponentSet& components);
	ArchetypeView();
public:
	class iterator
	{
		ArchetypeView& _view;
		size_t _chunkIndex = 0;
		size_t _entityIndex = 0;
		std::vector<byte*> components;
	public:
		iterator(ArchetypeView& view, size_t chunkIndex, size_t entityIndex);
		friend class Archetype;
		void operator++();
		bool operator!=(const iterator&) const;
		bool operator==(const iterator&) const;
		const std::vector<byte*>& operator*() const;

		using iterator_category = std::forward_iterator_tag;
		using reference = const std::vector<byte*>&;
	};
	friend class Archetype;

	iterator begin();
	iterator end();
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

	mutable shared_recursive_mutex _mutex;

	size_t chunkIndex(size_t entity) const;
	Chunk* getChunk(size_t entity) const;
public:
	Archetype(const std::vector<const ComponentDescription*>& components, std::shared_ptr<ChunkPool>& _chunkAllocator);
	~Archetype();
	bool hasComponent(ComponentID component) const;
	bool hasComponents(const ComponentSet& comps) const;
	VirtualComponent getComponent(size_t entity, ComponentID component) const;
	void setComponent(size_t entity, const VirtualComponent& component);
	void setComponent(size_t entity, const VirtualComponentView& component);
	bool isChildOf(const Archetype* parent, ComponentID& connectingComponent) const;
	const ComponentSet& components() const;
	const std::vector<const ComponentDescription*>& componentDescriptions();
	std::shared_ptr<ArchetypeEdge> getAddEdge(ComponentID component);
	std::shared_ptr<ArchetypeEdge> getRemoveEdge(ComponentID component);
	void addAddEdge(ComponentID component, Archetype* archetype);
	void addRemoveEdge(ComponentID component, Archetype* archetype);
	size_t size() const;
	size_t createEntity();
	size_t copyEntity(Archetype* source, size_t index);
	size_t entitySize() const;
	void remove(size_t index);

	void forEach(const std::vector<ComponentID>& components, const std::function<void(const byte* [])>& f, size_t start, size_t end);
	void forEach(const std::vector<ComponentID>& components, const std::function<void(byte* [])>& f, size_t start, size_t end);

	ArchetypeView getComponents(const ComponentSet& components);

	friend class ArchetypeView;
};

class ArchetypeManager;
class ArchetypeSet
{
	ArchetypeManager& _manager;
	std::vector<Archetype*> _archetypes;
public:
	class iterator
	{
		ArchetypeSet& _set;
		size_t _archetypeIndex = 0;
	public:
		iterator(ArchetypeSet& set, size_t index);
		void operator++();
		bool operator!=(const iterator&) const;
		bool operator==(const iterator&) const;
		Archetype& operator*();

		using iterator_category = std::forward_iterator_tag;
		using reference = Archetype&;
	};
	friend class ArchetypeManager;
	ArchetypeSet(ArchetypeManager& manager, const ComponentSet& components);
	iterator begin();
	iterator end();
};



class ArchetypeManager
{
#ifdef TEST_BUILD
public:
#endif

	std::shared_ptr<ChunkPool> _chunkAllocator;
	// Index 1: number of components, Index 2: archetype
	std::vector<std::vector<std::unique_ptr<Archetype>>> _archetypes;
	ComponentManager& _componentManager;
	friend class ArchetypeSet;
public:
	ArchetypeManager(ComponentManager& componentManager);
	Archetype* getArchetype(const ComponentSet& components);
	Archetype* makeArchetype(const ComponentSet& components);
	ArchetypeSet getArchetypes(const ComponentSet& components);

	void forEach(const std::vector<ComponentID>& components, const std::function <void(byte* [])>& f);
	void constForEach(const std::vector<ComponentID>& components, const std::function <void(const byte* [])>& f);
	std::shared_ptr<JobHandle> forEachParallel(const std::vector<ComponentID>& components, const std::function <void(byte* [])>& f, size_t entitiesPerThread);
	std::shared_ptr<JobHandle> constForEachParallel(const std::vector<ComponentID>& components, const std::function <void(const byte* [])>& f, size_t entitiesPerThread);

	void removeEmpty();
	void clear();
};
