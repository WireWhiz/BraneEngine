//
// Created by wirewhiz on 28/07/22.
//

#ifndef BRANEENGINE_ASSETEDITORCONTEXT_H
#define BRANEENGINE_ASSETEDITORCONTEXT_H

#include <vector>
#include "ecs/entityID.h"
#include "ecs/component.h"
#include <assets/assembly.h>

class Asset;
class AssetEditorContext
{
    struct Change
    {
        enum Type{
            change = 0,
            add = 1,
            erase = 2
        };
        Type type = change;
        size_t entityIndex = 0;
        Assembly::EntityAsset entityData;
    };

    Asset* _asset;
    std::vector<EntityID> _entityMap;

    size_t _currentChange = 0;
    std::vector<Change> _changes;
    void updateLinkedEntity(size_t entity);
public:
    size_t maxUndo = 200;
    AssetEditorContext(Asset* asset);
    ~AssetEditorContext();
    size_t addEntity();
    void updateEntity(size_t entity);
    void deleteEntity(size_t entity);
    void undo();
    void redo();
    Asset* asset();
    const std::vector<EntityID>& entities();
};


#endif //BRANEENGINE_ASSETEDITORCONTEXT_H
