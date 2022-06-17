//
// Created by wirewhiz on 4/13/22.
//

#include "runtime.h"
#include <utility/threadPool.h>

namespace Runtime
{
	std::unordered_map<std::string, std::unique_ptr<Module>> _modules;
	Timeline _timeline;
	std::atomic_bool _running = true;

	void addModule(Module* m)
	{
		_modules.insert({m->name(), std::unique_ptr<Module>(m)});
	}

	Timeline& timeline()
	{
		return _timeline;
	}

	void run()
	{
		for (auto& m : _modules)
		{
			m.second->start();
		}

		while (_running)
			_timeline.run();
	}

	void stop()
	{
		if(!_running)
			return;
		for (auto& m : _modules)
		{
			m.second->stop();
		}

		_running = false;
	}

	bool hasModule(const std::string& name)
	{
		return _modules.count(name);
	}

	Module* getModule(const std::string& name)
	{
		auto m = _modules.find(name);
		if(m == _modules.end())
			return nullptr;
		return m->second.get();
	}

	void init()
	{
		ThreadPool::init(4);
	}

	void cleanup()
	{
		stop();
		ThreadPool::cleanup();
	}
}