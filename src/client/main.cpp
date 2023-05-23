#include "client.h"

#include <config/config.h>
#include <networking/networking.h>
#include <assets/assetManager.h>
#include <fileManager/fileManager.h>
#include <runtimeServer/runtimeServer.h>

#include <chunk/chunkLoader.h>
#include <systems/transforms.h>
#include <graphics/graphics.h>

int main()
{

    Config::loadConfig();
    std::cout << "BraneSurfer starting up\n";

    Runtime::init();
    Timeline& tl = Runtime::timeline();
    tl.addBlock("asset management");
    tl.addBlock("networking");
    tl.addBlock("before main");
    tl.addBlock("main");
    tl.addBlock("draw");
    Runtime::addModule<FileManager>();
    Runtime::addModule<NetworkManager>();
    Runtime::addModule<EntityManager>();
    Runtime::addModule<AssetManager>();
    Runtime::addModule<ChunkLoader>();
    Runtime::addModule<graphics::VulkanRuntime>();
    Runtime::addModule<Transforms>();
   Runtime::addModule<Client>();

    Runtime::run();

    Runtime::cleanup();
    return 0;
}