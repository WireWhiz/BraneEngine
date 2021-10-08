#include "VirtualSystem.h"

VirtualSystem::VirtualSystem(SystemID id)
{
	_id = id;
}

SystemID VirtualSystem::id() const
{
	return _id;
}

FunctionPointerSystem::FunctionPointerSystem(SystemID id, SystemFunction function, void* data) : VirtualSystem(id)
{
	_id = id;
	_function = function;
	_data = data;
}

void FunctionPointerSystem::run(const EntityManager* em)
{
	_function(em, _data);
}
