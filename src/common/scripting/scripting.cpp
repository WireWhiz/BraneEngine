//
// Created by eli on 12/9/2022.
//

#include "scripting.h"

#include "ecs/entity.h"
#include "transforms.h"

ScriptManager::ScriptManager()
{
    _rt.setLinker(&_linker);
    /*addNativeComponent<EntityIDComponent>(em);
    addNativeComponent<EntityName>(em);
    addNativeComponent<Transform>(em);
    addNativeComponent<LocalTransform>(em);
    addNativeComponent<Children>(em);
    addNativeComponent<TRS>(em);
    addNativeComponent<MeshRendererComponent>(em);
    addNativeComponent<PointLightComponent>(em);
    addNativeComponent<graphics::Camera>(em);*/
    _linker.addType(EntityIDComponent::newTypeDef());
    _linker.addType(Transform::newTypeDef());
    _linker.addType(TRS::newTypeDef());
}

BraneScript::Compiler ScriptManager::newCompiler()
{
    return BraneScript::Compiler(&_linker);
}
