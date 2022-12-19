#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "byte.h"
#include "utility/inlineArray.h"
#include "entityID.h"
#include "runtime/runtime.h"
#include "typeDef.h"
#include "nativeTypes.h"

class InputSerializer;
class OutputSerializer;
class AssetID;

template <class T>
constexpr inline
T* getVirtual(const byte* var)
{
    return (T*)var;
}

template <class T>
constexpr inline
T readVirtual(byte* var)
{
    return *(T*)var;
}

template <class T>
constexpr inline
T readVirtual(const byte* var)
{
    return *(T*)var;
}

template <class T>
constexpr inline
void setVirtual(const byte* var, T value)
{
    *(T*)var = value;
}

namespace VirtualType
{
    template<typename T>
    BraneScript::ValueType type();
    template<typename T>
    std::string typeName();
    void serialize(BraneScript::ValueType type, OutputSerializer data, const byte* source);
    void deserialize(BraneScript::ValueType type, InputSerializer data, byte* source);
    size_t size(BraneScript::ValueType type);
    void construct(BraneScript::ValueType type, byte* var);
    void deconstruct(BraneScript::ValueType type, byte* var);
    void copy(BraneScript::ValueType type, byte* dest, const byte* source);
    void move(BraneScript::ValueType type, byte* dest, byte* source);
};

template<typename T>
BraneScript::ValueType VirtualType::type()
{
    if constexpr(std::is_same<T, bool>().value)
        return BraneScript::ValueType::Bool;
    if constexpr(std::is_same<T, int32_t>().value)
        return BraneScript::ValueType::Int32;
    if constexpr(std::is_same<T, uint32_t>().value)
        return BraneScript::ValueType::UInt32;
    if constexpr(std::is_same<T, int64_t>().value)
        return BraneScript::ValueType::Int64;
    if constexpr(std::is_same<T, uint64_t>().value)
        return BraneScript::ValueType::UInt64;
    if constexpr(std::is_same<T, float>().value)
        return BraneScript::ValueType::Float32;
    if constexpr(std::is_same<T, double>().value)
        return BraneScript::ValueType::Float64;
    return BraneScript::ValueType::Struct;
}

template<typename T>
std::string typeName()
{
    if constexpr(std::is_same<T, bool>().value)
        return "bool";
    if constexpr(std::is_same<T, int32_t>().value)
        return "int";
    if constexpr(std::is_same<T, uint32_t>().value)
        return "uint";
    if constexpr(std::is_same<T, int64_t>().value)
        return "int64";
    if constexpr(std::is_same<T, uint64_t>().value)
        return "uint64";
    if constexpr(std::is_same<T, float>().value)
        return "float";
    if constexpr(std::is_same<T, double>().value)
        return "double";
    if constexpr(std::is_same<T, std::string>().value)
        return "string";
    return "void";
}
