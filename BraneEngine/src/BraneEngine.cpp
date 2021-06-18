#include "BraneEngine.h"

using namespace std;

int main()
{
	//Run all our tests if we are in debug mode
#ifndef NDEBUG
	tests::runTests();
#endif // !NDEBUG


	return 0;
}
