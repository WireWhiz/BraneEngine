#pragma once
#include <cstdint>
#include "Component.h"
#include "VirtualSystem.h"
#include <memory>
#include <functional>
#include <unordered_map>
#include "../stackAllocate.h"

typedef uint64_t ArchetypeID;
class VirtualArchetype;

struct ArchetypeEdge
{
	ComponentID component;
	VirtualArchetype* archetype;
	ArchetypeEdge(ComponentID component, VirtualArchetype* archetype);
};

class VirtualArchetype
{
private:
	size_t _size = 0;
	std::vector<std::shared_ptr<ArchetypeEdge>> _addEdges;
	std::vector<std::shared_ptr<ArchetypeEdge>> _removeEdges;
public:
	std::unordered_map<ComponentID, std::unique_ptr<VirtualComponentVector>> components;
	VirtualArchetype(const std::vector<ComponentDefinition*>& componentDefs);
	bool hasComponent(ComponentID component) const;
	bool hasComponents(const std::vector<ComponentID>& comps) const;
	const VirtualComponentVector* getComponentVector(ComponentID component) const;
	bool isChildOf(const VirtualArchetype* parent, ComponentID& connectingComponent) const;
	bool isRootForComponent(ComponentID component);
	std::vector<ComponentDefinition*> getComponentDefs();
	std::shared_ptr<ArchetypeEdge> getAddEdge(ComponentID component);
	std::shared_ptr<ArchetypeEdge> getRemoveEdge(ComponentID component);
	void addAddEdge(ComponentID component, VirtualArchetype* archetype);
	void addRemoveEdge(ComponentID component, VirtualArchetype* archetype);
	void forAddEdge(const std::function<void(std::shared_ptr<ArchetypeEdge>)>& f);
	void forRemoveEdge(std::function<void(std::shared_ptr<ArchetypeEdge>)>& f);
	size_t size();
	size_t createEntity();
	size_t copyEntity(VirtualArchetype* source, size_t index);
	void swapRemove(size_t index);

	void forEach(const std::vector<ComponentID>& components, const std::function<void(byte* [])>& f);
}; 