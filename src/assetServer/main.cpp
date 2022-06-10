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
	Config::loadConfig();

	Runtime rt;
	Timeline& tl = rt.timeline();
	tl.addBlock("asset management");
	tl.addBlock("networking");
	tl.addBlock("before main");
	tl.addBlock("main");
	tl.addBlock("draw");
	rt.addModule<FileManager>();
	rt.addModule<NetworkManager>();
	rt.addModule<AssetManager>();
	rt.addModule<EntityManager>();
	rt.addModule<Database>();
	rt.addModule<AssetServer>();

	rt.run();
	return 0;
}