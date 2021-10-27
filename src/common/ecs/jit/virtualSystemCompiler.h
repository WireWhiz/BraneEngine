#pragma once
#include <asmjit/asmjit.h>

class VirtualSystemCompiler
{
public:
	int testFunction();
	int testFunction2();
	static int testCalledFunction(int a, int b)
	{
		return a + b;
	}
	const int c = 2;
	int testCalledFunction2(int a, int b)
	{
		return a + b + c;
	}
	static int callMember(VirtualSystemCompiler* thisPtr, int a, int b)
	{
  		return thisPtr->testCalledFunction2(a, b);
	}
};