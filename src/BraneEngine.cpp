#include "BraneEngine.h"
#include "core/VirtualSystemManager.h"

int main()
{
	//Run all our tests if we are in debug mode
#ifdef DEBUG
	tests::runTests();
#endif // !NDEBUG

	
	

#if defined( DEBUG) && defined(__linux__)
	int input;
	std::cin >> input;
#endif
	return 0;
}
