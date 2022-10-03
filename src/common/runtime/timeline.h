//
// Created by wirewhiz on 4/14/22.
//

#ifndef BRANEENGINE_TIMELINE_H
#define BRANEENGINE_TIMELINE_H

#include <forward_list>
#include <list>
#include "module.h"
#include <functional>
#include <memory>
#include <utility/staticIndexVector.h>
#include <string>

class ScheduledTask
{
    std::function<void()> _task;
    std::string _name;
public:
    [[nodiscard]] const std::string& name() const;
    ScheduledTask(const std::string& name, std::function<void()> task);
    void run();
};

class ScheduledBlock
{
    std::forward_list<ScheduledTask> _tasks;
    std::string _name;

public:
    explicit ScheduledBlock(const std::string& name);
    void run();
    [[nodiscard]] const std::string& name() const;

    void addTask(const ScheduledTask& runnable);
};

class Timeline
{
    std::list<ScheduledBlock> _blocks;
public:
    void addBlock(const std::string& name);
    void addBlockBefore(const std::string& name, const std::string& before);
    void addBlockAfter(const std::string& name, const std::string& before);
    ScheduledBlock* getTimeBlock(const std::string& name);
    void addTask(const std::string& name, std::function<void()> task, const std::string& timeBlock);
    void run();
};


#endif //BRANEENGINE_TIMELINE_H
