#include "assets/assetManager.h"
#include "config/config.h"
#include "ecs/entity.h"
#include "ecs/chunk.h"
#include "networking/networking.h"
#include "runtime/runtime.h"
#include "runtimeServer.h"

int main()
{
    Config::loadConfig();
    Runtime::init();
    Timeline& tl = Runtime::timeline();
    tl.addBlock("asset management");
    tl.addBlock("networking");
    tl.addBlock("before main");
    tl.addBlock("main");
    tl.addBlock("draw");
//    Runtime::addModule<NetworkManager>();
//    Runtime::addModule<AssetManager>();
//    Runtime::addModule<EntityManager>();
//    Runtime::addModule<ChunkManager>();
//    Runtime::addModule<RuntimeServer>();

    Runtime::cleanup();
    return 0;
}