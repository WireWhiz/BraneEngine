#pragma once
#include "test.h"
#include "ECS/tests.h"

namespace tests
{
	void runTests()
	{
		std::cout << "Running Tests" << std::endl;
		runECSTests();
		TestLogger::printFailedTests();
		TestLogger::throwIfFailed();
		if (TestLogger::testsSucceeded())
			std::cout << "Tests Succeeded" << std::endl << std::endl;
	}
}
