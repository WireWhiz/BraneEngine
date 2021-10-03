#include "VirtualSystem.h"

VirtualSystem::VirtualSystem(SystemID id)
{
	_id = id;
}

SystemID VirtualSystem::id() const
{
	return _id;
}

FunctionPointerSystem::FunctionPointerSystem(SystemID id, SystemFunction function) : VirtualSystem(id)
{
	_function = function;
}

void FunctionPointerSystem::run(const std::vector<byte*>& data, VirtualSystemGlobals* constants) const
{
	_function((byte**)data.data(), constants);
}
