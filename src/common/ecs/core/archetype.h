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

typedef uint64_t ArchetypeID;
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

	//Eventually move these to seperate node class
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
	bool isChildOf(const Archetype* parent, const ComponentAsset*& connectingComponent) const;
	bool isRootForComponent(const ComponentAsset* component) const;
	const ComponentSet& components() const;
	std::shared_ptr<ArchetypeEdge> getAddEdge(const ComponentAsset* component);
	std::shared_ptr<ArchetypeEdge> getRemoveEdge(const ComponentAsset* component);
	void addAddEdge(const ComponentAsset* component, Archetype* archetype);
	void addRemoveEdge(const ComponentAsset* component, Archetype* archetype);
	void forAddEdge(const std::function<void(std::shared_ptr<ArchetypeEdge>)>& f) const;
	void forRemoveEdge(std::function<void(std::shared_ptr<ArchetypeEdge>)>& f) const;
	size_t size();
	size_t createEntity();
	size_t copyEntity(Archetype* source, size_t index);
	const size_t entitySize();
	void remove(size_t index);

	void forEach(const ComponentSet& components, const std::function<void(const byte* [])>& f, size_t start, size_t end);
	void forEach(const ComponentSet& components, const std::function<void(byte* [])>& f, size_t start, size_t end);
}; 


typedef uint64_t EnityForEachID;

class ArchetypeManager
{
#ifdef TEST_BUILD
public:
#endif

	struct ForEachData
	{
		bool cached;
		ComponentSet components;
		std::vector<Archetype*> archetypeRoots;
		ForEachData(ComponentSet components);

	};

	mutable shared_recursive_mutex _forEachLock;
	std::vector<ForEachData> _forEachData;

	mutable shared_recursive_mutex _archetypeLock;
	std::unordered_map<const ComponentAsset*, std::vector<Archetype*>> _rootArchetypes;
	// Index 1: number of components, Index 2: archetype
	std::vector<std::vector<std::unique_ptr<Archetype>>> _archetypes;
	std::shared_ptr<ChunkPool> _chunkAllocator;

	void getRootArchetypes(const ComponentSet& components, std::vector<Archetype*>& roots) const;
	void updateForeachCache(const ComponentSet& components);
	std::vector<Archetype*>& getForEachArchetypes(EnityForEachID id);
	template<typename T>
	void forEachRecursive(Archetype* archetype, const ComponentSet& components, const std::function <T>& f, std::unordered_set<Archetype*>& executed)
	{
		if (executed.count(archetype))
			return;

		archetype->forEach(components, f, 0, archetype->size());
		executed.insert(archetype);

		archetype->forAddEdge([this, &components, &f, &executed](std::shared_ptr<ArchetypeEdge> edge) {
			forEachRecursive(edge->archetype, components, f, executed);
		});
	}

	template<typename T>
	void forEachRecursiveParellel(Archetype* archetype, const ComponentSet& components, const std::function <T>& f, std::unordered_set<Archetype*>& executed, size_t entitesPerThread, std::list<std::shared_ptr<JobHandle>>& jobs)
	{
		if (executed.count(archetype))
			return;
		size_t threads = archetype->size() / entitesPerThread + 1;

		for (size_t t = 0; t < threads; t++)
		{
			size_t start = t * entitesPerThread;
			size_t end = std::min(t * entitesPerThread + entitesPerThread, archetype->size());
			jobs.push_back(ThreadPool::enqueue([&]() {
				archetype->forEach(components, f, start, end);
			}));
		}

		executed.insert(archetype);

		archetype->forAddEdge([this, &components, &f, &executed, &entitesPerThread, &jobs](std::shared_ptr<ArchetypeEdge> edge) {
			forEachRecursiveParellel(edge->archetype, components, f, executed, entitesPerThread, jobs);
		});
	}


public:
	ArchetypeManager();
	Archetype* getArchetype(const ComponentSet& components);
	Archetype* makeArchetype(const ComponentSet& cdefs);
	EnityForEachID getForEachID(const ComponentSet& components);
	size_t forEachCount(EnityForEachID id);

	void forEach(EnityForEachID id, const std::function <void(byte* [])>& f);
	void constForEach(EnityForEachID id, const std::function <void(const byte* [])>& f);
	void forEachParellel(EnityForEachID id, const std::function <void(byte* [])>& f, size_t entitiesPerThread);
	void constForEachParellel(EnityForEachID id, const std::function <void(const byte* [])>& f, size_t entitiesPerThread);

};