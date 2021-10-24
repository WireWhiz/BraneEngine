#pragma once
#include <cstdint>
#include "component.h"
#include "virtualSystem.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include <common/utility/stackAllocate.h>

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
	size_t _entitySize = 0;

	//Eventually move these to seperate node class
	std::vector<std::shared_ptr<ArchetypeEdge>> _addEdges;
	std::vector<std::shared_ptr<ArchetypeEdge>> _removeEdges;
	//

	const ComponentSet _components;
	std::vector<std::unique_ptr<Chunk>> _chunks;

	std::shared_ptr<ChunkPool> _chunkAllocator;

	size_t chunkIndex(size_t entity) const;
public:
	Archetype(const ComponentSet& components, std::shared_ptr<ChunkPool>& _chunkAllocator);
	~Archetype();
	bool hasComponent(const ComponentAsset* component) const;
	bool hasComponents(const ComponentSet& comps) const;
	const VirtualComponentPtr getComponent(size_t entity, const ComponentAsset* component) const;
	byte* getComponent(size_t entity, size_t component) const;
	bool isChildOf(const Archetype* parent, const ComponentAsset*& connectingComponent) const;
	bool isRootForComponent(const ComponentAsset* component) const;
	const ComponentSet& components() const;
	std::shared_ptr<ArchetypeEdge> getAddEdge(const ComponentAsset* component);
	std::shared_ptr<ArchetypeEdge> getRemoveEdge(const ComponentAsset* component);
	void addAddEdge(const ComponentAsset* component, Archetype* archetype);
	void addRemoveEdge(const ComponentAsset* component, Archetype* archetype);
	void forAddEdge(const std::function<void(std::shared_ptr<ArchetypeEdge>)>& f);
	void forRemoveEdge(std::function<void(std::shared_ptr<ArchetypeEdge>)>& f);
	size_t size();
	size_t createEntity();
	size_t copyEntity(Archetype* source, size_t index);
	Chunk* getChunk(size_t entity) const;
	const size_t entitySize();
	void remove(size_t index);

	void forEach(const ComponentSet& components, const std::function<void(byte* [])>& f);
}; 
