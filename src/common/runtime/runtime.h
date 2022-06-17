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
	extern void init();
	extern void cleanup();
	extern void addModule(Module* m);
	template<typename T>
	inline void addModule()
	{
		static_assert(std::is_base_of<Module, T>().value);
		addModule(new T());
	}

	extern bool hasModule(const std::string& name);
	extern Module* getModule(const std::string& name);

	extern Timeline& timeline();

	extern void run();
	extern void stop();
};


#endif //BRANEENGINE_RUNTIME_H
