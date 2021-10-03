// Client
#include <iostream>
#include "tests/tests.h"
#include "client.h"

int main()
{
#ifdef DEBUG
	tests::runTests();
#endif

	Client c;
	c.run();

	return 0;
}