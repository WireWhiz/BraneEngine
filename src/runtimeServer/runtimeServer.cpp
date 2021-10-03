#include "runtimeServer.h"

RuntimeServer::RuntimeServer()
{
	_running = true;
}

RuntimeServer::~RuntimeServer()
{
}

void RuntimeServer::createSystems()
{
	//FunctionPointerSystem fps(0, );

}

void RuntimeServer::run()
{
	VirtualSystemGlobals vsg;
	while (_running)
	{
		em.runSystems(&vsg);
	}
}
