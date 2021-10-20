#include "Archetype.h"



Archetype::Archetype(const ComponentSet& componentDefs) : _componentDefs(componentDefs)
{
	_components.reserve(componentDefs.size());
	for (size_t i = 0; i < _componentDefs.size(); i++)
	{
		_components.push_back(VirtualComponentVector(_componentDefs.components()[i]));
	}

}

bool Archetype::hasComponent(const ComponentAsset* component) const
{
	return _componentDefs.contains(component);
}

bool Archetype::hasComponents(const ComponentSet& comps) const
{
	return _componentDefs.contains(comps);
}

const VirtualComponentPtr Archetype::getComponent(size_t entity, const ComponentAsset* component) const
{
	assert(entity < _size);
	return _components[_componentDefs.index(component)].getComponent(entity);
}

bool Archetype::isChildOf(const Archetype* parent, const ComponentAsset*& connectingComponent) const
{
	assert(_components.size() + 1 == parent->_components.size()); //Make sure this is a valid comparason
	byte missCount = 0;
	for (size_t i = 0; i - missCount < _componentDefs.size(); i++)
	{
		assert(i - missCount < _componentDefs.size());
			
		if (_componentDefs.components()[i - missCount] != parent->_componentDefs.components()[i])
		{
			connectingComponent = parent->_componentDefs.components()[i];
			if (++missCount > 1)
				return false;
		}
	}
	if(!connectingComponent)
		connectingComponent = parent->_componentDefs[parent->_componentDefs.size() - 1];
	return true;
}

bool Archetype::isRootForComponent(const ComponentAsset* component) const
{
	if (_componentDefs.size() == 1)
		return true;
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		if (_removeEdges[i]->component == component)
			return true;
	}
	return false;
}


const ComponentSet& Archetype::componentDefs() const
{
	return _componentDefs;
}

std::shared_ptr<ArchetypeEdge> Archetype::getAddEdge(const ComponentAsset* component)
{
	for (size_t i = 0; i < _addEdges.size(); i++)
	{
		if (component == _addEdges[i]->component)
			return _addEdges[i];
	}
	return nullptr;
}

std::shared_ptr<ArchetypeEdge> Archetype::getRemoveEdge(const ComponentAsset* component)
{
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		if (component == _removeEdges[i]->component)
			return _removeEdges[i];
	}
	return std::shared_ptr<ArchetypeEdge>();
}

void Archetype::addAddEdge(const ComponentAsset* component, Archetype* archetype)
{
	_addEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
}

void Archetype::addRemoveEdge(const ComponentAsset* component, Archetype* archetype)
{
	_removeEdges.push_back(std::make_shared<ArchetypeEdge>(component, archetype));
}

void Archetype::forAddEdge(const std::function<void(std::shared_ptr<ArchetypeEdge>)>& f)
{
	for (size_t i = 0; i < _addEdges.size(); i++)
	{
		f(_addEdges[i]);
	}
}

void Archetype::forRemoveEdge(std::function<void(std::shared_ptr<ArchetypeEdge>)>& f)
{
	for (size_t i = 0; i < _removeEdges.size(); i++)
	{
		f(_removeEdges[i]);
	}
}

size_t Archetype::size()
{
	return _size;
}

size_t Archetype::createEntity()
{
	for (size_t i = 0; i < _components.size(); i++)
	{
		_components[i].pushEmpty();
	}
	++_size;
	return _size - 1;
}

size_t Archetype::copyEntity(Archetype* source, size_t index)
{
	size_t newIndex = createEntity();
	for (size_t i = 0; i < _components.size(); i++)
	{
		if (_componentDefs[i]->size() == 0)
			continue;
		size_t sourceIndex = source->_componentDefs.index(_componentDefs.components()[i]);
		if (sourceIndex != nullindex)
			_components[i].copy(&source->_components[sourceIndex], index, newIndex);
		
	}
	return newIndex;
}

void Archetype::remove(size_t index)
{
	for (auto& c : _components)
	{
		c.remove(index);
	}
	_size--;
}

void Archetype::forEach(const ComponentSet& components, const std::function<void(byte* [])>& f)
{
	assert(components.size() > 0);
	// Small stack vector allocations are ok in some circumstances, for instance if this were a regular ecs system this function would probably be a template and use the same amount of stack memory
	{
		byte** data = (byte**)STACK_ALLOCATE(sizeof(byte*) * components.size());
		{ //Component Indicies is only needed in this scope
			size_t* componentIndicies = (size_t*)STACK_ALLOCATE(sizeof(size_t) * components.size());
			_componentDefs.indicies(components, componentIndicies);
			for (size_t i = 0; i < components.size(); i++)
			{
				assert(componentIndicies[i] >= 0 && componentIndicies[i] < _components.size());
				data[i] = _components[componentIndicies[i]].getComponentData(0);
			}
		}
		size_t* componentSizes = (size_t*)STACK_ALLOCATE(sizeof(size_t*) * components.size());
		for (size_t i = 0; i < components.size(); i++)
		{
			componentSizes[i] = components.components()[i]->size();
		}


		for (size_t entityIndex = 0; entityIndex < _size; entityIndex++)
		{
			f(data);
			for (size_t i = 0; i < components.size(); i++)
			{
				data[i] += componentSizes[i]; // Since we know the size of a struct, we can just increment by that
			}
		}
	}
	
}

ArchetypeEdge::ArchetypeEdge(const ComponentAsset* component, Archetype* archetype)
{
	assert(component && archetype);
	this->component = component;
	this->archetype = archetype;
}
