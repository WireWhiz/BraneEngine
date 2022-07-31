//
// Created by wirewhiz on 28/07/22.
//

#ifndef BRANEENGINE_ASSETEDITORCONTEXT_H
#define BRANEENGINE_ASSETEDITORCONTEXT_H

#include <vector>
#include "ecs/entityID.h"

class Asset;
class AssetEditorContext
{
    Asset* _asset;
    std::vector<EntityID> _entityMap;
public:
    AssetEditorContext(Asset* asset);
    ~AssetEditorContext();
    Asset* asset();
    const std::vector<EntityID>& entities();
};


#endif //BRANEENGINE_ASSETEDITORCONTEXT_H
