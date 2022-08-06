//
// Created by wirewhiz on 28/07/22.
//

#include "assetEditorContext.h"
#include "assets/assembly.h"
#include "ecs/entity.h"
#include "systems/transforms.h"
#include "assets/types/meshAsset.h"
#include "graphics/material.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "editor.h"
#include "graphics/graphics.h"

AssetEditorContext::AssetEditorContext(Asset* asset) : _asset(asset)
{
    if(_asset->type == AssetType::Type::assembly)
    {
        auto* em = Runtime::getModule<EntityManager>();
        EntityID root = em->createEntity(ComponentSet({EntityName::def()->id, Transform::def()->id, TRS::def()->id}));
        EntityName name;
        name.name = "EditorRoot";
        em->setComponent(root, name.toVirtual());
        auto entities = dynamic_cast<Assembly*>(_asset)->inject(*em, root);
        _entities.reserve(entities.size());
        for(auto id : entities)
            _entities.push_back({id, false});
    }
    else if(_asset->type == AssetType::Type::mesh)
    {
        auto* mesh = dynamic_cast<MeshAsset*>(_asset);
        auto* em = Runtime::getModule<EntityManager>();
        auto* editor = Runtime::getModule<Editor>();

        auto previewEntity = em->createEntity(ComponentSet({Transform::def()->id, MeshRendererComponent::def()->id, editor->defaultMaterial()->component()->id}));
        Transform t{};
        em->setComponent(previewEntity, t.toVirtual());

        if(mesh->pipelineID == -1)
            Runtime::getModule<graphics::VulkanRuntime>()->addMesh(mesh);

        MeshRendererComponent mr;
        mr.mesh = mesh->pipelineID;
        mr.materials.push_back(0);
        em->setComponent(previewEntity, mr.toVirtual());
        _entities.push_back({previewEntity, false});
    }
    _changes.reserve(maxUndo);
}

Asset* AssetEditorContext::asset()
{
    return _asset;
}

const std::vector<AssetEditorContext::EditorEntity>& AssetEditorContext::entities()
{
    return _entities;
}

AssetEditorContext::~AssetEditorContext()
{
    auto* em = Runtime::getModule<EntityManager>();
    for(auto e : _entities)
    {
        em->destroyEntity(e.id);
    }
}

void AssetEditorContext::updateEntity(size_t entity)
{
    assert(entity < _entities.size());

    auto* assembly = dynamic_cast<Assembly*>(_asset);
    assert(assembly);

    if(_currentChange + 1 < _changes.size())
        _changes.erase(_changes.begin() + _currentChange + 1, _changes.end());

    if(!_entities[entity].unsavedChanges)
        _changes.push_back({Change::initialValue, true, entity, assembly->entities[entity]});

    auto* em = Runtime::getModule<EntityManager>();
    for(auto& comp : assembly->entities[entity].components)
    {
        VirtualComponent currentValue = em->getComponent(_entities[entity].id, comp.description()->id);
        if(currentValue.description() == Transform::def())
            currentValue.setVar(1, true);
        comp = referencesToAsset(currentValue);
    }

    _changes.push_back({Change::change, !_entities[entity].unsavedChanges, entity, assembly->entities[entity]});
    _entities[entity].unsavedChanges = true;
    _currentChange = _changes.size() - 1;
}

void AssetEditorContext::undo()
{
    if(_currentChange == 0)
        return;
    auto& change = _changes[--_currentChange];

    auto* assembly = dynamic_cast<Assembly*>(_asset);
    assert(assembly);

    switch(change.type)
    {
        case Change::initialValue:
            assembly->entities[change.entIndex] = change.entityData;
            updateLinkedEntity(change.entIndex);
            _entities[change.entIndex].unsavedChanges = false;
            undo();
            return;
        case Change::change:
        case Change::removeComponent:
            assembly->entities[change.entIndex] = change.entityData;
            updateLinkedEntity(change.entIndex);
            return;
        default:
            Runtime::error("Unimplemented change type: " + std::to_string(change.type));
            assert(false);
    }
}

void AssetEditorContext::redo()
{
    if(_currentChange == _changes.size() - 1)
        return;
    auto& change = _changes[++_currentChange];

    auto* assembly = dynamic_cast<Assembly*>(_asset);
    assert(assembly);

    switch(change.type)
    {
        case Change::initialValue:
            assembly->entities[change.entIndex] = change.entityData;
            updateLinkedEntity(change.entIndex);
            _entities[change.entIndex].unsavedChanges = true;
            redo();
            return;
        case Change::change:
        case Change::removeComponent:
            assembly->entities[change.entIndex] = change.entityData;
            updateLinkedEntity(change.entIndex);
            return;
        default:
            Runtime::error("Unimplemented change type: " + std::to_string(change.type));
            assert(false);
    }
}

void AssetEditorContext::updateLinkedEntity(size_t entity)
{
    auto* assembly = dynamic_cast<Assembly*>(_asset);
    assert(assembly);
    auto* em = Runtime::getModule<EntityManager>();

    EntityID& id = _entities[entity].id;
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
        if(_entities[i].id == id)
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
            comp.setVar<EntityID>(m, _entities[comp.getVar<EntityID>(m)->id].id);
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
    auto* em = Runtime::getModule<EntityManager>();
    auto& components = assembly->entities[entity].components;
    VirtualComponentView component = components[componentIndex];

    if(!_entities[entity].unsavedChanges)
        _changes.push_back({Change::initialValue, true, entity, assembly->entities[entity]});

    if(em->hasComponent(_entities[entity].id, component.description()->id))
        em->removeComponent(_entities[entity].id, component.description()->id);
    components.erase(components.begin() + componentIndex);

    _changes.push_back({Change::removeComponent, !_entities[entity].unsavedChanges, entity, assembly->entities[entity]});
    _entities[entity].unsavedChanges = true;
    _currentChange = _changes.size() - 1;
}
