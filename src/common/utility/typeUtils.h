#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "byte.h"
#include "utility/inlineArray.h"
#include "entityID.h"
#include "runtime/runtime.h"
#include "scriptRuntime/typeDef.h"

class InputSerializer;
class OutputSerializer;
class AssetID;

namespace TypeUtils
{
    void serialize(const BraneScript::TypeDef* type, OutputSerializer data, const byte* source);
    void deserialize(const BraneScript::TypeDef* type, InputSerializer data, byte* source);
    void construct(const BraneScript::TypeDef* type, byte* var);
    void deconstruct(const BraneScript::TypeDef* type, byte* var);
    void copy(const BraneScript::TypeDef* type, byte* dest, const byte* source);
    void move(const BraneScript::TypeDef* type, byte* dest, byte* source);

}

