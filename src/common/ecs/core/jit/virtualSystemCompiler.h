#pragma once
#include <asmjit/asmjit.h>

class VirtualSystemCompiler
{
public:
	void testFunction();
	static int testCalledFunction(int a, int b)
	{
		return a + b;
	}
};