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

namespace Runtime
{
	void init();
	void cleanup();
	void addModule(const std::string& name, Module* m);
	template<typename T>
	void addModule()
	{
		static_assert(std::is_base_of<Module, T>());
		addModule(T::name(), new T());
	}

	bool hasModule(const std::string& name);
	Module* getModule(const std::string& name);
	template<typename T>
	T* getModule()
	{
		static_assert(std::is_base_of<Module, T>());
		return (T*)getModule(T::name());
	}


	Timeline& timeline();

	void run();
	void stop();
};


#endif //BRANEENGINE_RUNTIME_H
