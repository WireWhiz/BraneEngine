//
// Created by wirewhiz on 25/12/22.
//

#include "typeSerializer.h"
//#include "structDefinition.h"
#include "robin_hood.h"

/*
namespace TypeSerializer
{
    robin_hood::unordered_map<const TypeDef*, CustomSerializer> _customSerializers;

    void addCustomSerializer(const TypeDef* type, CustomSerializer serializer)
    {
        assert(!_customSerializers.contains(type));
        _customSerializers.insert({type, serializer});
    }

    void serialize(const byte* data, const TypeDef* type, OutputSerializer serializer)
    {
        if(type->type() == BraneScript::ValueType::Struct)
        {
            auto* sd = static_cast<const BraneScript::StructDef*>(type);
            for(auto& m : sd->memberVars())
                serialize(data + m.offset, m.type, serializer);
            return;
        }

        switch(type->type())
        {
            case BraneScript::Bool:
                serializer << *(bool*)data;
                return;
            case BraneScript::Int32:
                serializer << *(int32_t*)data;
                return;
            case BraneScript::Int64:
                serializer << *(int64_t*)data;
                return;
            case BraneScript::Float32:
                serializer << *(float*)data;
                return;
            case BraneScript::Float64:
                serializer << *(double*)data;
                return;
            default:
                auto s = _customSerializers.find(type);
                if(s == _customSerializers.end())
                    throw std::runtime_error("serialization type error");
                s->second.serialize(data, serializer);
        }
    }

    void deserialize(byte* data, const TypeDef* type, InputSerializer serializer)
    {
        if(type->type() == BraneScript::ValueType::Struct)
        {
            auto* sd = static_cast<const BraneScript::StructDef*>(type);
            for(auto& m : sd->memberVars())
                deserialize(data + m.offset, m.type, serializer);
            return;
        }

        switch(type->type())
        {
            case BraneScript::Bool:
                serializer >> *(bool*)data;
                return;
            case BraneScript::Int32:
                serializer >> *(int32_t*)data;
                return;
            case BraneScript::Int64:
                serializer >> *(int64_t*)data;
                return;
            case BraneScript::Float32:
                serializer >> *(float*)data;
                return;
            case BraneScript::Float64:
                serializer >> *(double*)data;
                return;
            default:
                auto s = _customSerializers.find(type);
                if(s == _customSerializers.end())
                    throw std::runtime_error("deserialization type error");
                s->second.deserialize(data, serializer);
        }
    }

    Json::Value toJson(const byte* data, const TypeDef* type)
    {
        Json::Value output;
        if(type->type() == BraneScript::ValueType::Struct)
        {
            auto* sd = static_cast<const BraneScript::StructDef*>(type);
            for(auto& m : sd->memberVars())
                output[m.name] = toJson(data + m.offset, m.type);
            return std::move(output);
        }

        switch(type->type())
        {
            case BraneScript::Bool:
                output = *(bool*)data;
                break;
            case BraneScript::Int32:
                output = *(int32_t*)data;
                break;
            case BraneScript::Int64:
                output = *(int64_t*)data;
                break;
            case BraneScript::Float32:
                output = *(float*)data;
                break;
            case BraneScript::Float64:
                output = *(double*)data;
                break;
            default:
                auto s = _customSerializers.find(type);
                if(s == _customSerializers.end())
                    throw std::runtime_error("serialization type error");
                output = s->second.toJson(data);
        }
        return std::move(output);
    }

    void fromJson(byte* data, const TypeDef* type, const Json::Value& source)
    {
        if(type->type() == BraneScript::ValueType::Struct)
        {
            auto* sd = static_cast<const BraneScript::StructDef*>(type);
            for(auto& m : sd->memberVars())
                fromJson(data + m.offset, m.type, source[m.name]);
        }

        switch(type->type())
        {
            case BraneScript::Bool:
                if(source.isBool())
                    *(bool*)data = source.asBool();
                else
                    *(bool*)data = false;
                break;
            case BraneScript::Int32:
                if(source.isInt())
                    *(int32_t*)data = source.asInt();
                else
                    *(int32_t*)data = 0;
                break;
            case BraneScript::Int64:
                if(source.isInt64())
                    *(int64_t*)data = source.asInt64();
                else
                    *(int64_t*)data = 0;
                break;
            case BraneScript::Float32:
                if(source.isDouble())
                    *(float*)data = source.asFloat();
                else
                    *(float*)data = 0.0f;
                break;
            case BraneScript::Float64:
                if(source.isDouble())
                    *(double*)data = source.asDouble();
                else
                    *(double*)data = 0.0;
                break;
            default:
                auto s = _customSerializers.find(type);
                if(s == _customSerializers.end())
                    throw std::runtime_error("serialization type error");
                s->second.fromJson(data, source);
        }
    }
};*/
