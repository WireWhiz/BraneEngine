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
            initialValue = 0,
            change = 1,
            add = 2,
            erase = 3
        };
        Type type = change;
        bool firstChange = false;
        size_t entIndex;
        Assembly::EntityAsset entityData;
    };

    struct EditorEntity
    {
        EntityID id;
        bool unsavedChanges = false;
    };

    Asset* _asset;
    std::vector<EditorEntity> _entities;

    size_t _currentChange = 0;
    std::vector<Change> _changes;
    void updateLinkedEntity(size_t entity);
    EntityID entityIndex(EntityID id);
    VirtualComponent& referencesToAsset(VirtualComponent& entity);
    VirtualComponent& referencesToGlobal(VirtualComponent& entity);
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
    const std::vector<EditorEntity>& entities();
};


#endif //BRANEENGINE_ASSETEDITORCONTEXT_H
