#pragma once
#include "virtualType.h"
#include "component.h"
#include <vector>

class EntityManager;
typedef uint64_t SystemID;
// argument 1 is reference to the different variables in the struct
// argument 2 is constant values and native functions
using SystemFunction = void (*)(EntityManager*, void*);

class VirtualSystem
{
protected:
	SystemID _id;
public:
	VirtualSystem(SystemID id);
	SystemID id() const;
	virtual void run(EntityManager* em) = 0;
};

class FunctionPointerSystem : VirtualSystem
{
	SystemFunction _function;
	void* _data;
public:
	FunctionPointerSystem(SystemID id, SystemFunction function, void* data);
	void run(EntityManager* em) override;
};