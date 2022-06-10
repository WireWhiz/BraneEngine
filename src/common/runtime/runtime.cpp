//
// Created by wirewhiz on 4/13/22.
//

#include "runtime.h"
#include <utility/threadPool.h>

void Runtime::addModule(Module* m)
{
	_modules.insert({m->name(), std::unique_ptr<Module>(m)});
}

Timeline& Runtime::timeline()
{
	return _timeline;
}

void Runtime::run()
{
	for (auto& m : _modules)
	{
		m.second->start();
	}

	while (_running)
		_timeline.run();
}

void Runtime::stop()
{
	if(!_running)
		return;
	for (auto& m : _modules)
	{
		m.second->stop();
	}

	_running = false;
}

bool Runtime::hasModule(const std::string& name)
{
	return _modules.count(name);
}

Module* Runtime::getModule(const std::string& name)
{
	auto m = _modules.find(name);
	if(m == _modules.end())
		return nullptr;
	return m->second.get();
}

Runtime::Runtime()
{
	ThreadPool::init(4);
}

Runtime::~Runtime()
{
	stop();
	ThreadPool::cleanup();
}
