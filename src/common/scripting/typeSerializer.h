//
// Created by wirewhiz on 25/12/22.
//

#ifndef BRANEENGINE_TYPESERIALIZER_H
#define BRANEENGINE_TYPESERIALIZER_H

/*#include "typeDef.h"
#include "json/json.h"
#include "utility/serializedData.h"
#include "functionHandle.h"

using BraneScript::TypeDef;

namespace TypeSerializer
{
    struct CustomSerializer
    {
        BraneScript::FunctionHandle<void, const byte*, OutputSerializer> serialize = nullptr;
        BraneScript::FunctionHandle<void, byte*, InputSerializer>        deserialize = nullptr;
        BraneScript::FunctionHandle<Json::Value, const byte*>            toJson = nullptr;
        BraneScript::FunctionHandle<void, byte*, const Json::Value&>     fromJson = nullptr;
    };
    void addCustomSerializer(const TypeDef* type, CustomSerializer serializer);
    void serialize(const byte* data, const TypeDef* type, OutputSerializer serializer);
    void deserialize(byte* data, const TypeDef* type, InputSerializer serializer);
    Json::Value toJson(const byte* data, const TypeDef* type);
    void fromJson(byte* data, const TypeDef* type, const Json::Value& source);
};*/

#endif //BRANEENGINE_TYPESERIALIZER_H
