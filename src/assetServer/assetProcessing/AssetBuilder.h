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
	static void extractNodes_recursive(gltfLoader& loader, uint32_t nodeIndex, uint32_t parentIndex, std::vector<WorldEntity>& entities);
public:
	static std::vector<MeshAsset*>  extractMeshesFromGltf(gltfLoader& loader);
	static std::vector<WorldEntity> extractNodes(gltfLoader& loader);
};


#endif //BRANEENGINE_ASSETBUILDER_H
