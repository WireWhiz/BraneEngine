#include "virtualSystemBlock.h"
#include "entity.h"

SystemBlock::SystemBlock(const std::string& identifier, const std::string& after, const std::string& before)
{
	this->identifier = identifier;
	this->after = after;
	this->before = before;
}

void SystemBlock::setNext(SystemBlock* next)
{
	_next = next;
}

SystemBlock* SystemBlock::next() const
{
	return _next;
}

void SystemBlock::runSystems(const EntityManager* entities) const
{
	for (size_t i = 0; i < _systems.size(); i++)
	{
		_systems[i]->run(entities);
	}
}

void SystemBlock::addSystem(VirtualSystem* system)
{
	_systems.push_back(std::unique_ptr<VirtualSystem>(system));
}

void SystemBlock::removeSystem(SystemID id)
{
	for (size_t i = 0; i < _systems.size(); i++)
	{
		if (_systems[i]->id() == id)
		{
			_systems[i] = std::move(_systems[_systems.size() - 1]);
			_systems.resize(_systems.size() - 1);
			break;
		}
	}
	assert(false); // system not found
}

VirtualSystem* SystemBlock::getSystem(SystemID id) const
{
	for (size_t i = 0; i < _systems.size(); i++)
	{
		if (_systems[i]->id() == id)
			return _systems[i].get();
	}
	assert(false); // system not found
	return nullptr;
}

bool SystemBlockList::sort()
{
	for (auto& node : _nodes)
	{
		node.second->sorted = false;
		node.second->visited = false;
	}
	SystemBlock* last = nullptr;
	try
	{
		for (auto& node : _nodes)
			node.second->sort(last, _first);
	}
	catch (const insertion_error& error)
	{
		return false;
	}
	return true;

}

SystemBlockList::SystemBlockNode* SystemBlockList::findNode(const std::string& identifier) const
{
	if (_nodes.count(identifier))
		return _nodes.find(identifier)->second.get();
	else
		return nullptr;
}

SystemBlock* SystemBlockList::operator[](size_t index) const

{
	if (index >= _nodes.size() || index < 0)
		throw new std::out_of_range("System Block List index was out of range");

	SystemBlock* currentBlock = _first;
	for (size_t i = 0; i < index; i++)
		currentBlock = currentBlock->next();

	return currentBlock;
}

SystemBlock* SystemBlockList::find(const std::string& identifier)
{
	return _nodes[identifier]->block.get();
}

bool SystemBlockList::addBlock(const std::string& identifier, const std::string& after, const std::string& before)
{
	SystemBlock* newBlock = new SystemBlock(identifier, after, before);
	SystemBlockNode* newNode = new SystemBlockNode(newBlock);

	// Add any edges defined by this block
	if (before != "")
	{
		SystemBlockNode* linkedNode = findNode(before);
		if (linkedNode != nullptr)
		{
			linkedNode->addAfter(newNode);
		}
	}
	if (after != "")
	{
		SystemBlockNode* linkedNode = findNode(after);
		if (linkedNode != nullptr)
		{
			newNode->addAfter(linkedNode);
		}
	}

	// Find any other references to this block
	for (auto& node : _nodes)
	{
		SystemBlock* block = node.second->block.get();
		if (block->after == identifier && block->identifier != before)
		{
			node.second->addAfter(newNode);
		}
		else if (block->before == identifier && block->identifier != after)
		{
			newNode->addAfter(node.second.get());
		}
	}


	_nodes[identifier] = (std::unique_ptr<SystemBlockNode>(newNode));
	if (!sort())
	{
		_nodes.erase(identifier); // remove what we just added
		sort();
		return false;
	}
	return true;
}

void SystemBlockList::removeBlock(const std::string& identifier)
{
	
}

size_t SystemBlockList::size() const
{
	return _nodes.size();
}

void SystemBlockList::runSystems(const EntityManager* em)
{
	SystemBlock* current = _first;
	while (current)
	{
		current->runSystems(em);
		current = current->next();
	}
}

void SystemBlockList::SystemBlockNode::sort(SystemBlock*& last, SystemBlock*& _first)
{
	if (sorted)
		return;
	if (visited)
		throw insertion_error("Circular dependancy found when sorting block nodes");

	visited = true;
	for (size_t i = 0; i < _after.size(); i++)
	{
		_after[i]->sort(last, _first);
	}
	visited = false;
	
	if (last != nullptr)
		last->setNext(block.get());
	else
		_first = block.get();
		
	last = block.get();

	sorted = true;
}


void SystemBlockList::SystemBlockNode::addAfter(SystemBlockNode* node)
{
	_after.push_back(node);
}