//
// Created by wirewhiz on 28/07/22.
//

#ifndef BRANEENGINE_ASSETEDITORCONTEXT_H
#define BRANEENGINE_ASSETEDITORCONTEXT_H

#include <vector>
#include "ecs/entityID.h"
#include "ecs/component.h"
#include "assets/assembly.h"
#include "utility/serializedData.h"

class Asset;
class AssetEditorContext
{
    struct Change
    {
        virtual ~Change() = default;
        virtual void applyChange(Asset* asset) = 0;
        virtual void undoChange(Asset* asset) = 0;
    };

    template<typename T>
    struct SerializedFieldChange : public Change
    {
        void* field = nullptr;
        SerializedData before;
        SerializedData after;
        void applyChange(Asset* asset) override
        {
            InputSerializer s(after);
            s >> *(T*)(field);
        }
        void undoChange(Asset* asset) override
        {
            InputSerializer s(before);
            s >> *(T*)(field);
        }
    };

    struct EntityAssetChange : public Change
    {
        size_t index;
    };

    struct EntityAssetDataChange : public EntityAssetChange
    {
        Assembly::EntityAsset before;
        Assembly::EntityAsset after;
        void applyChange(Asset* asset) override;
        void undoChange(Asset* asset) override;
    };

    struct DeleteEntityAssetChange : public EntityAssetChange
    {
        Assembly::EntityAsset before;
        void applyChange(Asset* asset) override;
        void undoChange(Asset* asset) override;
    };

    struct AddEntityAssetChange : public EntityAssetChange
    {
        void applyChange(Asset* asset) override;
        void undoChange(Asset* asset) override;
    };

    Asset* _asset;
    bool _displayPreviewEntities = false;
    std::vector<EntityID> _entities;
    bool _dirty = false;

    std::deque<std::unique_ptr<Change>>::iterator _currentChange;
    std::deque<std::unique_ptr<Change>> _changes;
    void updateLinkedEntity(size_t entity);
    EntityID entityIndex(EntityID id);
    VirtualComponent& referencesToAsset(VirtualComponent& entity);
    VirtualComponent& referencesToGlobal(VirtualComponent& entity);
    void pushChange(std::unique_ptr<Change> change);
public:
    size_t maxUndo = 200;
    AssetEditorContext(Asset* asset);
    ~AssetEditorContext();

    template<typename T>
    void setSerializedField(T* assetVariable, const T& value)
    {
        auto change = std::make_unique<SerializedFieldChange<T>>();
        change->field = assetVariable;
        OutputSerializer before(change->before);
        before << *assetVariable;
        *assetVariable = value;
        OutputSerializer after(change->after);
        after << *assetVariable;
        pushChange(std::move(change));
    }

    size_t addEntity();
    void updateEntity(size_t entity);
    void addComponent(size_t entity, const ComponentDescription* component);
    void removeComponent(size_t entity, uint32_t component);
    void deleteEntity(size_t entity);
    void undo();
    void redo();
    bool unsavedChanges() const;
    void setPreviewEntities(bool visible);
    void save();
    Asset* asset();
    const std::vector<EntityID>& entities();
};


#endif //BRANEENGINE_ASSETEDITORCONTEXT_H
