//
// Created by wirewhiz on 4/14/22.
//

#ifndef BRANEENGINE_TIMELINE_H
#define BRANEENGINE_TIMELINE_H

#include <forward_list>
#include "module.h"
#include <functional>
#include <memory>
#include <utility/staticIndexVector.h>

class ScheduledTask
{
    std::function _task;
public:
    ScheduledTask(const std::string& name, std::function task);
    void run();
};

class ScheduledBlock
{
    std::forward_list<ScheduledTask> _tasks;
    std::string _name;

public:
    ScheduledBlock(const std::string& name);
    std::vector<std::string> dependencies;
    ScheduledBlock(const std::string& name);
    void run();
    const std::string& name() const;

    void addTask(const ScheduledTask& runnable);
};

class Timeline
{
    struct BlockNode
    {
        bool visited;
        bool sorted;
        std::unique_ptr<ScheduledBlock> block;
        BlockNode* next;
        bool sort(BlockNode*& first, BlockNode*& last, std::unordered_map<std::string, BlockNode>& blocks)
    };
    BlockNode* _first;

    std::unordered_map<std::string, BlockNode> _blocks;
    bool sortNodes();
public:
    bool addBlock(ScheduledBlock* block);
    bool addDependency(const std::string& before, const std::string& after);
    ScheduledBlock* getTimeBlock(const std::string& name);
    void addTask(const std::string& name, std::function<void()> task, const std::string& timeBlock);
};


#endif //BRANEENGINE_TIMELINE_H
