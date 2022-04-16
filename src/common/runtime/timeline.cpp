//
// Created by wirewhiz on 4/14/22.
//

#include "timeline.h"

bool Timeline::BlockNode::sort(Timeline::BlockNode*& first, Timeline::BlockNode*& last,
                               std::unordered_map<std::string, BlockNode>& blocks)
{
    if(sorted)
        return true;
    if(visited)
        return false;

    visited = true;
    for(auto& d : block->dependencies)
    {
        auto node = blocks.find(d);
        if(node == blocks.end())
            return false;
        node->second.sort(first, last, blocks);
    }

    if(last)
        last->next = this;
    else
    {
        first = this;
        last = this;
    }

    visited = false;

    return true;
}

bool Timeline::sortNodes()
{
    for(auto& n : _blocks)
    {
        n.second.next = nullptr;
        n.second.visited = false;
        n.second.sorted = false;
    }

    BlockNode* last;
    for(auto& n : _blocks)
    {
        if(!n.second.sort(_first, last, _blocks))
            return false;
    }

    return true;
}

bool Timeline::addBlock(ScheduledBlock* block)
{
    BlockNode bn{};
    bn.block = std::unique_ptr<ScheduledBlock>(block);

    _blocks.insert({block->name(), std::move(bn)});
    if(!sortNodes())
    {
        _blocks.erase(block->name());
        sortNodes();
        return false;
    }
    return true;
}

bool Timeline::addDependency(const std::string& before, const std::string& after)
{
    assert(_blocks.count(before));
    assert(_blocks.count(after));

    auto a = _blocks.find(after);
    if(a == _blocks.end() || !_blocks.count(before))
        return false;

    a->second.block->dependencies.push_back(before);

    if(!sortNodes())
    {
        a->second.block->dependencies.resize(a->second.block->dependencies.size() - 1);
        sortNodes();
        return false;
    }
    return true;
}

ScheduledBlock* Timeline::getTimeBlock(const std::string& name)
{
    auto i = _blocks.find(name);
    if(i == _blocks.end())
        return nullptr;
    return i->second.block.get();
}

void Timeline::addTask(const std::string& name, std::function<void()> task, const std::string& timeBlock)
{
    _blocks.find(timeBlock)->second.block->addTask(ScheduledTask(name, task));
}


