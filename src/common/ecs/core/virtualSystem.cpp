#include "VirtualSystem.h"

VirtualSystem::VirtualSystem(SystemID id)
{
	_id = id;
}

SystemID VirtualSystem::id() const
{
	return _id;
}