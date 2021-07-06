#pragma once
#include "VirtualType.h"
#include <cstdint>

typedef uint64_t SystemID;
// argument 1 is reference to struct
// argument 2 is constants values and native functions
using SystemFunction = void (*)(byte*, byte*);

class VirtualSystem
{
	SystemFunction behaviour;
};    