#pragma once
#include "VirtualType.h"
#include "Component.h"
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <system_error>

struct VirtualSystemGlobals
{
	int test;
};

typedef uint64_t SystemID;
// argument 1 is reference to struct
// argument 2 is constant values and native functions
using SystemFunction = void (*)(byte*, VirtualSystemGlobals*);

class VirtualSystem
{
protected:
	SystemID _id;
public:
	VirtualSystem(SystemID id)
	{
		_id = id;
	}
	SystemID id() const
	{
		return _id;
	}
	virtual void run(const std::vector<byte*> data, VirtualSystemGlobals* constants) const = 0;
	virtual const std::vector<ComponentID> requiredComponents() const = 0;
};

