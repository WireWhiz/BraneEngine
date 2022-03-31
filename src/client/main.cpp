// Client
#include <iostream>
#include "client.h"
#include <utility/threadPool.h>

int main()
{

	Config::loadConfig();
	Client c;
	c.run();
	return 0;
}