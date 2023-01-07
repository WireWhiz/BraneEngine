#pragma once

#ifdef _WIN32
#define STACK_ALLOCATE(x) _alloca(x)
#else

#include <alloca.h>

#define STACK_ALLOCATE(x) alloca(x)
#endif