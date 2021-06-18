#pragma once
#include <ecs.h>

namespace tests
{
	void testVirtualStructs()
	{
		TestLogger::setTestCatagory("Virtual Structs");
		expectError(throw std::runtime_error("test error"), std::runtime_error);
		//expectError(std::cout << "No, I won't do it!" << std::endl , std::runtime_error);
		expectValue(2, 2);
		//expectValue(2, 1);
	}

	void runECSTests()
	{
		testVirtualStructs();
	}
}