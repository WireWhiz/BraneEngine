
#include <iostream>
#include "assets/assetManager.h"
#include "graphics/graphics.h"
#include "client.h"

int main()
{

	Config::loadConfig();
	std::cout << "BraneSurfer starting up\n";

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
	Runtime::addModule<graphics::VulkanRuntime>();
	Runtime::addModule<Client>();

	Runtime::run();
	return 0;
}