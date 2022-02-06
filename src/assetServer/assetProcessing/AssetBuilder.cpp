//
// Created by eli on 2/1/2022.
//

#include "AssetBuilder.h"

std::vector<MeshAsset*> AssetBuilder::extractMeshesFromGltf(gltfLoader& loader)
{

	return loader.extractAllMeshes();
}
