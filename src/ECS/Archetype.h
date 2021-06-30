#pragma once
#include <cstdint>
#include <Component.h>
#include <memory>
typedef uint64_t ArchetypeID;

class VirtualArchetype;

struct ArchetypeEdge
{
	ComponentID component;
	size_t addIndex;
	size_t removeIndex;
};

class VirtualArchetype
{
private:
	std::vector<std::shared_ptr<ArchetypeEdge>> _edges;
public:
	VirtualArchetype(const std::vector<ComponentDefinition*>& componentDefs);
	std::vector<VirtualComponentVector> components;
	bool hasComponent(ComponentID component) const;
	const VirtualComponentVector* getComponentVector(ComponentID component) const;
	bool isChildOf(const VirtualArchetype& parent, ComponentID& connectingComponent) const;
	std::vector<ComponentDefinition*> getComponentDefs();
	std::shared_ptr<ArchetypeEdge> getEdge(ComponentID component);
	void addEdge(std::shared_ptr<ArchetypeEdge> edge);
	size_t createEntity();
	size_t copyEntity(VirtualArchetype* source, size_t index);
	void swapRemove(size_t index);
}; 