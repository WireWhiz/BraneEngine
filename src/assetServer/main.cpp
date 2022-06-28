// Asset server
#include <common/config/config.h>
#include <ecs/ecs.h>
#include <fileManager/fileManager.h>
#include "assetServer.h"
#include <database/Database.h>
#include <runtime/runtime.h>

int main()
{
	Runtime::init();
	Config::loadConfig();

	Timeline& tl = Runtime::timeline();
	tl.addBlock("asset management");
	tl.addBlock("networking");
	tl.addBlock("before main");
	tl.addBlock("main");
	tl.addBlock("draw");
	Runtime::addModule<FileManager>();
	Runtime::addModule<NetworkManager>();
	Runtime::addModule<AssetManager>();
	Runtime::addModule<EntityManager>();
	Runtime::addModule<Database>();
	Runtime::addModule<AssetServer>();

	Runtime::run();
	Runtime::cleanup();
	return 0;
}