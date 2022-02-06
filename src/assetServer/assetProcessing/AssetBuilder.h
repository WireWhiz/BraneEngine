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
	static std::vector<MeshAsset*>  extractMeshesFromGltf(gltfLoader& loader);
};


#endif //BRANEENGINE_ASSETBUILDER_H
