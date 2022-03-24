//
// Created by eli on 3/19/2022.
//

#ifndef BRANEENGINE_GRAPHICASSETPREPROCESSORS_H
#define BRANEENGINE_GRAPHICASSETPREPROCESSORS_H
#include "assets/assetManager.h"
#include "assets/assembly.h"
#include "assets/types/meshAsset.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "graphics.h"

class GraphicAssetPreprocessors
{
public:
	static void addAssetPreprocessors(AssetManager& am, graphics::VulkanRuntime& vkr);
};


#endif //BRANEENGINE_GRAPHICASSETPREPROCESSORS_H
