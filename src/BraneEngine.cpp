#include "BraneEngine.h"
#include "core/VirtualSystemManager.h"

int main()
{
	//Run all our tests if we are in debug mode
#ifdef DEBUG
	tests::runTests();
#endif // !NDEBUG

	// Create JitCompiler
	// Create runtime
	// Create native systems
	// load new systems and component types
	// run systmes
	// occasionally check if components and systems are being used, if they arn't, deallocate them.
	SystemBlockList blocks;


	//one
	//two
	//three
	//four
	//five
	blocks.addBlock("after after one", "after one", "");
	blocks.addBlock("before two",      "",          "two");
	blocks.addBlock("one",             "",          "");
	blocks.addBlock("four",            "three",       "");
	blocks.addBlock("three",           "two",          "four");
	blocks.addBlock("after one",       "one",       "");
	blocks.addBlock("two",             "one",       "three");
	for (size_t i = 0; i < blocks.size(); i++)
	{
		std::cout << blocks[i]->identifier << std::endl;
	}

#if defined( DEBUG) && defined(__linux__)
	int input;
	std::cin >> input;
#endif
	return 0;
}
