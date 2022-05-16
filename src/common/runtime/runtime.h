//
// Created by wirewhiz on 4/13/22.
//

#ifndef BRANEENGINE_RUNTIME_H
#define BRANEENGINE_RUNTIME_H

#include <unordered_map>
#include <string>
#include "timeline.h"
#include <atomic>

class Module;

class Runtime
{
protected:
    std::unordered_map<std::string, std::unique_ptr<Module>> _modules;
    Timeline _timeline;
	std::atomic_bool _running = true;

public:
	Runtime();
	~Runtime();
    void addModule(Module* m);
	template<typename T>
	void addModule()
	{
		static_assert(std::is_base_of<Module, T>().value);
		addModule(new T(*this));
	}

    bool hasModule(const std::string& name);
    Module* getModule(const std::string& name);

	Timeline& timeline();

	void run();
	void stop();
};


#endif //BRANEENGINE_RUNTIME_H
