#pragma once
#ifdef DEBUG
#include "testing.h"
#include "ECS/ECSTests.h"

namespace tests
{
	void runTests()
	{
		std::cout << "Running Tests" << std::endl;
		runECSTests();
		TestRunner::printFailedTests();
		TestRunner::throwIfFailed();
		if (TestRunner::testsSucceeded())
			std::cout << "Tests Succeeded" << std::endl << std::endl;
	}
}
#endif