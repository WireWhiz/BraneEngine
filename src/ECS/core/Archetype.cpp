#include "Archetype.h"

VirtualArchetype::VirtualArchetype(const std::vector<ComponentDefinition*>& componentDefs)
{
	for (size_t i = 0; i < componentDefs.size(); i++)
	{
		components.insert({ componentDefs[i]->id(), std::make_unique<VirtualComponentVector>(componentDefs[i]) });
	}
}

bool VirtualArchetype::hasComponent(ComponentID component) const
{
	return components.find(component) != components.end();
}

bool VirtualArchetype::hasComponents(const std::vector<ComponentID>& comps) const
{
	for (size_t i = 0; i < comps.size(); i++)
	{
		if (components.find(comps[i]) == components.end())
			return false;
	}
	return true;
}

const VirtualComponentVector* VirtualArchetype::getComponentVector(ComponentID component) const
{
	auto vec = components.find(component);
	if (vec != components.end())
		return vec->second.get();
	else
		return nullptr;
}

bool VirtualArchetype::isChildOf(const VirtualArchetype* parent, ComponentID& connectingComponent) const
{
	assert(components.size() + 1 == parent->components.size()); //Make sure this is a valid comparason

	byte miss_count = 0;
	for (auto& c : parent->components)
	{
		if (!hasComponent(c.first))
		{
			connectingComponent = c.first;
			if (++miss_count > 1)
				return false;
		}
	}
	
	return true;
}

bool VirtualArchetype::isRootForComponent(ComponentID component)
{
	size_t edges = _removeEdges.size();
	if(edges > 1)
	{
		return false;
	}
	else if (edges == 1)
	{
		return _removeEdges[0]->component == component;
	}
	return true;
}


std::vector<ComponentDefinition*> VirtualArchetype::getComponentDefs()
{
	std::vector<ComponentDefinition*> componentDefs;
	for (auto& c : components)
	{
		componentDefs.push_back(c.second->def());
	}
	return componentDefs;
}

std::shared_ptr<ArchetypeEdge> VirtualArchetype::getAddEdge(ComponentID component)
{
	for (size_t i = 0; i < _addEdges.size(); i++)
	{
		if (component == _addEdges[i]->component)
			return _addEdges[i];
	}
	return std::shared_ptr<ArchetypeEdge>();
}

std::shared_ptr<ArchetypeEdge> VirtualArchetype::getRemoveEdge(ComponentID component)
{
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		if (component == _removeEdges[i]->component)
			return _removeEdges[i];
	}
	return std::shared_ptr<ArchetypeEdge>();
}

void VirtualArchetype::addAddEdge(ComponentID component, VirtualArchetype* archetype)
{
	_addEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
}

void VirtualArchetype::addRemoveEdge(ComponentID component, VirtualArchetype* archetype)
{
	_removeEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
}

void VirtualArchetype::forAddEdge(const std::function<void(std::shared_ptr<ArchetypeEdge>)>& f)
{
	for (size_t i = 0; i < _addEdges.size(); i++)
	{
		f(_addEdges[i]);
	}
}

void VirtualArchetype::forRemoveEdge(std::function<void(std::shared_ptr<ArchetypeEdge>)>& f)
{
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		f(_removeEdges[i]);
	}
}

size_t VirtualArchetype::createEntity()
{
	size_t index;
	for (auto& c : components)
	{
		index = c.second->size();
		c.second->pushEmpty();
	}
	return index;
}

size_t VirtualArchetype::copyEntity(VirtualArchetype* source, size_t index)
{
	size_t newIndex = createEntity();
	for (auto& c : components)
	{
		auto sourceComponent = source->components.find(c.first);
		if (sourceComponent != source->components.end())
		{
			c.second->copy(sourceComponent->second.get(), index, newIndex);
		}
	}
	return newIndex;
}

void VirtualArchetype::swapRemove(size_t index)
{
	for (auto& c : components)
	{
		c.second->swapRemove(index);
	}
}

void VirtualArchetype::forEach(const std::vector<ComponentID>& requiredComponents, const std::function<void(byte* [])>& f)
{
	std::vector<byte*> data;
	data.resize(requiredComponents.size(), nullptr);

	std::vector<VirtualComponentVector*> requiredComponentVectors;
	for (size_t i = 0; i < requiredComponents.size(); i++)
	{
		requiredComponentVectors.push_back(components[requiredComponents[i]].get());
	}

	for (size_t entityIndex = 0; entityIndex < requiredComponentVectors[0]->size(); entityIndex++)
	{
		for (size_t i = 0; i < requiredComponents.size(); i++)              
		{
			data[i] = requiredComponentVectors[i]->getComponentData(entityIndex);
		}
		f(data.data());
	}
	
}

ArchetypeEdge::ArchetypeEdge(ComponentID component, VirtualArchetype* archetype)
{
	this->component = component;
	this->archetype = archetype;
}
