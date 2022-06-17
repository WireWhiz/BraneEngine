// Asset server
#include <iostream>
#include <common/config/config.h>
#include <json/json.h>
#include <fstream>
#include <string>
#include <ecs/ecs.h>
#include <fileManager/fileManager.h>
#include "assetNetworking/assetServer.h"
#include <assets/types/meshAsset.h>
#include <assets/types/shaderAsset.h>
#include <database/Database.h>
#include <runtime/runtime.h>

/*struct SentMesh : public NativeComponent<SentMesh>
{
	REGISTER_MEMBERS_0("Sent Mesh");
};*/

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