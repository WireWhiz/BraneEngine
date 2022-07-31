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
        _entityMap = dynamic_cast<Assembly*>(_asset)->inject(*em, root);
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
        _entityMap.push_back(previewEntity);
    }
}

Asset* AssetEditorContext::asset()
{
    return _asset;
}

const std::vector<EntityID>& AssetEditorContext::entities()
{
    return _entityMap;
}

AssetEditorContext::~AssetEditorContext()
{
    auto* em = Runtime::getModule<EntityManager>();
    for(auto e : _entityMap)
    {
        em->destroyEntity(e);
    }
}
