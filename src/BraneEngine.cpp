#include "BraneEngine.h"

using namespace std;

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


	return 0;
}
