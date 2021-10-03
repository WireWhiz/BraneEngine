#include "../testing.h"
#include <common/ecs/jit/VirtualSystemCompiler.h>

namespace tests
{
	void runJITTests()
	{
		VirtualSystemCompiler vsc;
		vsc.testFunction();
	}
}