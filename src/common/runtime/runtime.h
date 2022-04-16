//
// Created by wirewhiz on 4/13/22.
//

#ifndef BRANEENGINE_RUNTIME_H
#define BRANEENGINE_RUNTIME_H

#include <unordered_map>
#include <string>
#include "timeline.h"

class Module;

class Runtime
{
protected:
    std::unordered_map<std::unique_ptr<Module>> _modules;
    Timeline _timeline;

public:

    void addModule(Module* m);
    void hasModule(const std::string& name);
    Module* getModule(const std::string& name);

    template<typename T>
    T* hasModule();
    template<typename T>
    T* getModule();
};


#endif //BRANEENGINE_RUNTIME_H
