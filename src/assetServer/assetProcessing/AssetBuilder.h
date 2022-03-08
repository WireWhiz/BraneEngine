//
// Created by eli on 2/1/2022.
//

#ifndef BRANEENGINE_ASSETBUILDER_H
#define BRANEENGINE_ASSETBUILDER_H

#include "assets/assembly.h"
#include "assets/types/meshAsset.h"
#include "../gltf/gltfLoader.h"

class AssetBuilder
{
public:
	static std::vector<std::unique_ptr<Asset>> buildAssembly(const std::string& name, gltfLoader& loader);
};




#endif //BRANEENGINE_ASSETBUILDER_H
