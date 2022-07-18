//
// Created by eli on 1/21/2022.
//

#ifndef BRANEENGINE_CHUNK_H
#define BRANEENGINE_CHUNK_H
#include "asset.h"
#include <ecs/core/component.h>
#include "assembly.h"

class WorldChunk : public Assembly
{
public:
	std::vector<AssetID> dependencies; // Any systems in dependencies will be automatically loaded
	std::vector<EntityAsset> data;
};


#endif //BRANEENGINE_CHUNK_H
