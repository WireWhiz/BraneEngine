// Client
#include <iostream>
#include "client.h"
#include <utility/threadPool.h>

int main()
{
	ThreadPool::init(4);
	Config::loadConfig();
	Client c;
	c.run();

	ThreadPool::cleanup();
	return 0;
}