#pragma once
#include <cstdint>
#include "component.h"
#include "virtualSystem.h"
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
	const ComponentAsset* component;
	Archetype* archetype;
	ArchetypeEdge(const ComponentAsset* component, Archetype* archetype);
};

template <size_t N> class ChunkBase;
typedef ChunkBase<16384> Chunk;
class ChunkPool;

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

	const ComponentSet _components;
	std::vector<std::unique_ptr<Chunk>> _chunks;
	std::shared_ptr<ChunkPool> _chunkAllocator;

	mutable shared_recursive_mutex _mutex;

	size_t chunkIndex(size_t entity) const;
	Chunk* getChunk(size_t entity) const;
public:
	Archetype(const ComponentSet& components, std::shared_ptr<ChunkPool>& _chunkAllocator);
	~Archetype();
	bool hasComponent(const ComponentAsset* component) const;
	bool hasComponents(const ComponentSet& comps) const;
	VirtualComponent getComponent(size_t entity, const ComponentAsset* component) const;
	void setComponent(size_t entity, const VirtualComponent& component);
	void setComponent(size_t entity, const VirtualComponentPtr& component);
	bool isChildOf(const Archetype* parent, const ComponentAsset*& connectingComponent) const;
	const ComponentSet& components() const;
	std::shared_ptr<ArchetypeEdge> getAddEdge(const ComponentAsset* component);
	std::shared_ptr<ArchetypeEdge> getRemoveEdge(const ComponentAsset* component);
	void addAddEdge(const ComponentAsset* component, Archetype* archetype);
	void addRemoveEdge(const ComponentAsset* component, Archetype* archetype);
	void forAddEdge(const std::function<void(std::shared_ptr<const ArchetypeEdge>)>& f) const;
	void forRemoveEdge(std::function<void(std::shared_ptr<const ArchetypeEdge>)>& f) const;
	size_t size() const;
	size_t createEntity();
	size_t copyEntity(Archetype* source, size_t index);
	size_t entitySize() const;
	void remove(size_t index);

	void forEach(const ComponentSet& components, const std::function<void(const byte* [])>& f, size_t start, size_t end);
	void forEach(const ComponentSet& components, const std::function<void(byte* [])>& f, size_t start, size_t end);
}; 


typedef uint64_t EntityForEachID;

class ArchetypeManager
{
#ifdef TEST_BUILD
public:
#endif

	struct ForEachData
	{
		ComponentSet components;
		ComponentSet exclude;
		std::vector<Archetype*> archetypes;
		ForEachData(ComponentSet components, ComponentSet exclude);

	};

	std::vector<ForEachData> _forEachData;

	// Index 1: number of components, Index 2: archetype
	std::vector<std::vector<std::unique_ptr<Archetype>>> _archetypes;
	std::shared_ptr<ChunkPool> _chunkAllocator;

	void findArchetypes(const ComponentSet& components, const ComponentSet& exclude, std::vector<Archetype*>& archetypes) const;
	void cacheArchetype(Archetype* arch);
	void removeCachedArchetype(Archetype* arch); //TODO create archetype cleanup system
	std::vector<Archetype*>& getForEachArchetypes(EntityForEachID id);

	/*
	template<typename T>
	void forEachRecursive(Archetype* archetype, const ComponentSet& components, const std::function <T>& f, std::unordered_set<Archetype*>& executed)
	{
		if (executed.count(archetype))
			return;

		archetype->forEach(components, f, 0, archetype->size());
		executed.insert(archetype);

		archetype->forAddEdge([this, &components, &f, &executed](std::shared_ptr<const ArchetypeEdge> edge) {
			forEachRecursive(edge->archetype, components, f, executed);
		});
	}

	template<typename T>
	void forEachRecursiveParallel(Archetype* archetype, const ComponentSet& components, const std::function <T>& f, std::unordered_set<Archetype*>& executed, size_t entitiesPerThread, std::shared_ptr<JobHandle> handle)
	{
		if (executed.count(archetype))
			return;
		size_t threads = archetype->size() / entitiesPerThread + 1;

		for (size_t t = 0; t < threads; t++)
		{
			size_t start = t * entitiesPerThread;
			size_t end = std::min(t * entitiesPerThread + entitiesPerThread, archetype->size());
			ThreadPool::enqueue([archetype, &f, &components, start, end]() {
				archetype->forEach(components, f, start, end);
			}, handle);
		}

		executed.insert(archetype);

		archetype->forAddEdge([this, &components, &f, &executed, &entitiesPerThread, &handle](std::shared_ptr<const ArchetypeEdge> edge) {
			forEachRecursiveParallel(edge->archetype, components, f, executed, entitiesPerThread, handle);
		});
	}
	*/

public:
	ArchetypeManager();
	Archetype* getArchetype(const ComponentSet& components);
	Archetype* makeArchetype(const ComponentSet& components);

	EntityForEachID getForEachID(const ComponentSet& components, const ComponentSet& exclude = ComponentSet());
	size_t forEachCount(EntityForEachID id);

	void forEach(EntityForEachID id, const std::function <void(byte* [])>& f);
	void constForEach(EntityForEachID id, const std::function <void(const byte* [])>& f);
	std::shared_ptr<JobHandle> forEachParallel(EntityForEachID id, const std::function <void(byte* [])>& f, size_t entitiesPerThread);
	std::shared_ptr<JobHandle> constForEachParallel(EntityForEachID id, const std::function <void(const byte* [])>& f, size_t entitiesPerThread);

};