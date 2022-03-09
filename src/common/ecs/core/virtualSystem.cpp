#include "virtualSystem.h"

VirtualSystem::VirtualSystem(AssetID id, SystemFunction function)
{
	id = id;
	_function = function;
}

void VirtualSystem::run(EntityManager& em)
{
	_function(em);
}
