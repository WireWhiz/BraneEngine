//
// Created by eli on 4/22/2022.
//

#include <graphics/graphics.h>
#include <runtime/runtime.h>
#include <runtime/module.h>
#include "assets/assetManager.h"
#include "fileManager/fileManager.h"
#include "networking/networking.h"
#include "ui/editorUI.h"

int main()
{
	Runtime rt;
	Timeline& tl = rt.timeline();
	tl.addBlock("asset management");
	tl.addBlock("networking");
	tl.addBlock("before main");
	tl.addBlock("main");
	tl.addBlock("draw");
	rt.addModule<NetworkManager>();
	rt.addModule<FileManager>();
	rt.addModule<AssetManager>();
	rt.addModule<EntityManager>();
	rt.addModule<graphics::VulkanRuntime>();
	rt.addModule<EditorUI>();

	rt.run();
}