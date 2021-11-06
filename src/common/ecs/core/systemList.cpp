#include "systemList.h"
#include "entity.h"

SystemList::SystemNode::SystemNode(std::unique_ptr<VirtualSystem>&& system)
{
	_system = std::move(system);
	_id = _system->id();
	_next = nullptr;
	sorted = false;
	visited = false;
}

void SystemList::SystemNode::setNext(SystemNode* next)
{
	_next = next;
}

SystemList::SystemNode* SystemList::SystemNode::next() const
{
	return _next;
}

void SystemList::SystemNode::run(EntityManager* entities) const
{
	_system->run(entities);
}

VirtualSystem* SystemList::SystemNode::system() const
{
	return _system.get();
}

SystemID SystemList::SystemNode::id() const
{
	return _system->id();
}

bool SystemList::sort()
{
	for (auto& node : _nodes)
	{
		node.second->sorted = false;
		node.second->visited = false;
		node.second->setNext(nullptr);
	}
	_first = nullptr;
	
	for (auto& node : _nodes)
		if (!node.second->sort(_first))
			return false;
	return true;

}

void SystemList::updateNodeConstraints(SystemNode* node)
{
	for (auto& nodeContainer : _nodes)
	{
		SystemNode* otherNode = nodeContainer.second.get();
		for (SystemID beforeConstraint : otherNode->beforeConstraints)
		{
			if (beforeConstraint == node->id())
				node->after.push_back(otherNode);
		}

		for (SystemID afterConstraint : node->afterConstraints)
		{
			if (afterConstraint == otherNode->id())
				otherNode->after.push_back(node);
		}
	}
}

void SystemList::removeNode(SystemID id)
{
	assert(_nodes.count(id));
	SystemNode* node = _nodes[id].get();
	for (auto& nodeContainer : _nodes)
	{
		SystemNode* otherNode = nodeContainer.second.get();
		for (size_t i = 0; i < otherNode->after.size(); i++)
		{
			if (node == otherNode->after[i])
			{
				size_t last = otherNode->after.size() - 1;
				otherNode->after[i] = otherNode->after[last];
				otherNode->after.resize(last);
				break;
			}
		}
	}
}

SystemList::SystemList()
{
	_first = nullptr;
}

VirtualSystem* SystemList::findSystem(SystemID id) const
{
	auto systemNode = _nodes.find(id);
	if (systemNode != _nodes.end())
		return systemNode->second->system();
	else
		return nullptr;
}

bool SystemList::addSystem(std::unique_ptr<VirtualSystem>&& system)
{
	SystemNode* newNode = new SystemNode(std::move(system));

	// Find any other references to this block
	updateNodeConstraints(newNode);


	_nodes[newNode->id()] = std::unique_ptr<SystemNode>(newNode);
	if (!sort())
	{
		removeNode(newNode->id()); // remove what we just added
		sort();
		return false;
	}
	return true;
}

void SystemList::removeSystem(SystemID id)
{
	assert(_nodes.count(id));
	removeNode(id);
}

bool SystemList::addBeforeConstraint(SystemID id, SystemID before)
{
	assert(_nodes.count(id));
	SystemNode* node = _nodes[id].get();

	node->beforeConstraints.push_back(before);

	auto beforeNode = _nodes.find(before);
	if (beforeNode != _nodes.end())
		beforeNode->second->after.push_back(node);
	if (!sort())
	{
		if (beforeNode != _nodes.end())
			beforeNode->second->after.resize(beforeNode->second->after.size() - 1);
		node->beforeConstraints.resize(node->beforeConstraints.size() - 1);
		bool result = sort();
		assert(result);
		return false;
	}
	return true;
}

bool SystemList::addAfterConstraint(SystemID id, SystemID after)
{
	assert(_nodes.count(id));
	SystemNode* node = _nodes[id].get();

	node->afterConstraints.push_back(after);

	auto afterNode = _nodes.find(after);
	if (afterNode != _nodes.end())
		node->after.push_back(afterNode->second.get());
	if (!sort())
	{
		if (afterNode != _nodes.end())
			node->after.resize(node->after.size() - 1);
		node->afterConstraints.resize(node->afterConstraints.size() - 1);
		bool result = sort();
		assert(result);
		return false;
	}
	return true;
}

size_t SystemList::size() const
{
	return _nodes.size();
}

void SystemList::runSystems(EntityManager* em)
{

	SystemNode* current = _first;
	while (current)
	{
		current->run(em);
		current = current->next();
	}
}

bool SystemList::SystemNode::sort(SystemNode*& last)
{
	if (sorted)
		return true;
	if (visited)
		return false;

	visited = true;
	for (size_t i = 0; i < after.size(); i++)
	{
		if (!after[i]->sort(last))
			return false;
	}
	visited = false;
	
	if (last != nullptr)
		last->setNext(this);
	else
		last = this;

	sorted = true;
	return true;
}