//
// Created by wirewhiz on 28/07/22.
//

#include "assetEditorContext.h"
#include "assets/assembly.h"
#include "ecs/entity.h"
#include "systems/transforms.h"
#include "assets/types/meshAsset.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "editor.h"
#include "graphics/graphics.h"
#include "tinyfiledialogs.h"
#include "serverFilesystem.h"

AssetEditorContext::AssetEditorContext(Asset* asset) : _asset(asset)
{
    _currentChange = _changes.begin();
}

Asset* AssetEditorContext::asset()
{
    return _asset;
}

const std::vector<EntityID>& AssetEditorContext::entities()
{
    return _entities;
}

AssetEditorContext::~AssetEditorContext()
{
    setPreviewEntities(false);
    if(_dirty)
    {
        if(tinyfd_messageBox("Save Asset", ("Save changes to " + _asset->name + "?").c_str(), "yesno", "question", 1))
        {
            save();
        }
        else
        {
            Runtime::warn("Discarded changes to " + _asset->name);
        }
    }
}

void AssetEditorContext::updateEntity(size_t entity)
{
    assert(entity < _entities.size());
    assert(_displayPreviewEntities);

    auto* assembly = dynamic_cast<Assembly*>(_asset);
    if(!assembly)
        return;

    auto change = std::make_unique<EntityAssetDataChange>();

    change->index = entity;
    change->before = assembly->entities[entity];

    auto* em = Runtime::getModule<EntityManager>();
    for(auto& comp : assembly->entities[entity].components)
    {
        VirtualComponent currentValue = em->getComponent(_entities[entity], comp.description()->id);
        if(currentValue.description() == Transform::def())
            currentValue.setVar(1, true);
        comp = referencesToAsset(currentValue);
    }

    change->after = assembly->entities[entity];
    pushChange(std::move(change));
}

void AssetEditorContext::undo()
{
    if(_currentChange == _changes.begin())
        return;
    auto& change = *(--_currentChange);

    change->undoChange(_asset);
    auto* entityChange = dynamic_cast<EntityAssetChange*>(change.get());
    if(entityChange)
        updateLinkedEntity(entityChange->index);
    _dirty = true;
}

void AssetEditorContext::redo()
{
    if(_currentChange == _changes.end())
        return;
    auto& change = *(_currentChange);

    change->applyChange(_asset);
    auto* entityChange = dynamic_cast<EntityAssetChange*>(change.get());
    if(entityChange)
        updateLinkedEntity(entityChange->index);
    _dirty = true;
    ++_currentChange;
}

void AssetEditorContext::updateLinkedEntity(size_t entity)
{
    if(!_displayPreviewEntities)
        return;
    auto* assembly = dynamic_cast<Assembly*>(_asset);
    assert(assembly);
    auto* em = Runtime::getModule<EntityManager>();

    EntityID& id = _entities[entity];
    auto& entityAsset = assembly->entities[entity];
    // -1 accounts for entity ID component
    auto* arch = em->getEntityArchetype(id);
    if(arch->components().size() - 1 != entityAsset.components.size())
    {
        for(auto& c : arch->components())
        {
            if(c != EntityIDComponent::def()->id)
                em->removeComponent(id, c);
        }
        for(auto& c : entityAsset.components)
            em->addComponent(id, c.description()->id);
    }


    for(auto& comp : entityAsset.components)
    {
        VirtualComponent copy = comp;
        em->setComponent(id, referencesToGlobal(copy));
        em->markComponentChanged(id, comp.description()->id);
    }
}

EntityID AssetEditorContext::entityIndex(EntityID id)
{
    for(uint32_t i = 0; i < _entities.size(); ++i)
        if(_entities[i] == id)
            return {i, static_cast<uint32_t>(-1)};
    return {0, static_cast<uint32_t>(-1)};
}

VirtualComponent& AssetEditorContext::referencesToAsset(VirtualComponent& comp)
{
    auto* assembly = dynamic_cast<Assembly*>(_asset);
    assert(assembly);
    auto& members = comp.description()->members();
    for(size_t m = 0; m < members.size(); ++m)
    {
        if(members[m].type == VirtualType::virtualEntityID)
            comp.setVar<EntityID>(m, entityIndex(*comp.getVar<EntityID>(m)));
        else if(members[m].type == VirtualType::virtualEntityIDArray)
        {
            auto& entityIDs = *comp.getVar<inlineEntityIDArray>(m);
            for(auto& id : entityIDs)
            {
                id = entityIndex(id);
            }
        }
    }
    return comp;
}

VirtualComponent& AssetEditorContext::referencesToGlobal(VirtualComponent& comp)
{
    auto* assembly = dynamic_cast<Assembly*>(_asset);
    assert(assembly);
    auto& members = comp.description()->members();
    for(size_t m = 0; m < members.size(); ++m)
    {
        if(members[m].type == VirtualType::virtualEntityID)
            comp.setVar<EntityID>(m, _entities[comp.getVar<EntityID>(m)->id]);
        else if(members[m].type == VirtualType::virtualEntityIDArray)
        {
            auto& entityIDs = *comp.getVar<inlineEntityIDArray>(m);
            for(auto& id : entityIDs)
                id = _entities[id.id].id;
        }
    }
    return comp;
}

void AssetEditorContext::removeComponent(size_t entity, uint32_t componentIndex)
{
    auto* assembly = dynamic_cast<Assembly*>(_asset);
    assert(assembly);
    auto& components = assembly->entities[entity].components;
    VirtualComponentView component = components[componentIndex];

    auto change = std::make_unique<EntityAssetDataChange>();
    change->before = assembly->entities[entity];
    change->index = componentIndex;

    if(_displayPreviewEntities)
    {
        auto* em = Runtime::getModule<EntityManager>();
        if(em->hasComponent(_entities[entity], component.description()->id))
            em->removeComponent(_entities[entity], component.description()->id);
    }
    components.erase(components.begin() + componentIndex);

    change->after = assembly->entities[entity];
    pushChange(std::move(change));
}

void AssetEditorContext::pushChange(std::unique_ptr<Change> change)
{
    _changes.erase(_currentChange, _changes.end());
    _changes.push_back(std::move(change));
    while(_changes.size() > maxUndo)
        _changes.pop_front();
    _currentChange = _changes.end();
    _dirty = true;
}

bool AssetEditorContext::unsavedChanges() const
{
    return _dirty;
}

void AssetEditorContext::save()
{
    _dirty = false;
    Runtime::log("Saving " + _asset->name);
    auto* fs = Runtime::getModule<ServerFilesystem>();
    fs->updateAsset(_asset);
}

void AssetEditorContext::setPreviewEntities(bool visible)
{
    if(_displayPreviewEntities == visible)
        return;
    auto* em = Runtime::getModule<EntityManager>();
    if(visible)
    {
        if(_asset->type == AssetType::Type::assembly)
        {
            EntityID root = em->createEntity(ComponentSet({EntityName::def()->id, Transform::def()->id, TRS::def()->id}));
            EntityName name;
            name.name = "EditorRoot";
            em->setComponent(root, name.toVirtual());
            _entities = dynamic_cast<Assembly*>(_asset)->inject(*em, root);
        }
        else if(_asset->type == AssetType::Type::mesh)
        {
            auto* mesh = dynamic_cast<MeshAsset*>(_asset);

            auto previewEntity = em->createEntity(ComponentSet({Transform::def()->id, MeshRendererComponent::def()->id}));
            Transform t{};
            em->setComponent(previewEntity, t.toVirtual());

            MeshRendererComponent mr;
            mr.mesh = mesh->runtimeID;
            mr.materials.push_back(0);
            em->setComponent(previewEntity, mr.toVirtual());
            _entities.push_back(previewEntity);
        }
    }
    else
    {
        for(auto e : _entities)
        {
            em->destroyEntity(e);
        }
    }
    _displayPreviewEntities = visible;
}

void AssetEditorContext::EntityAssetDataChange::applyChange(Asset* asset)
{
    dynamic_cast<Assembly*>(asset)->entities[index] = after;
}

void AssetEditorContext::EntityAssetDataChange::undoChange(Asset* asset)
{
    dynamic_cast<Assembly*>(asset)->entities[index] = before;
}
