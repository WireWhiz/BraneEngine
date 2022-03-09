#pragma once
#include "virtualType.h"
#include "component.h"
#include "assets/assetID.h"
#include <functional>

class EntityManager;
// argument 1 is reference to the different variables in the struct
// argument 2 is constant values and native functions
using SystemFunction = std::function< void (EntityManager&)>;

class VirtualSystem
{
protected:
	SystemFunction _function;
public:
	AssetID id;
	VirtualSystem(AssetID id, SystemFunction function);
	virtual void run(EntityManager& em);
};
