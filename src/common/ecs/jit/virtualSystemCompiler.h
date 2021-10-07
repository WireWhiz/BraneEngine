#pragma once
#include <asmjit/asmjit.h>

class VirtualSystemCompiler
{
public:
	int testFunction();
	static int testCalledFunction(int a, int b)
	{
		return a + b;
	}
};