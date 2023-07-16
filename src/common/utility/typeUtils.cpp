#include "typeUtils.h"
#include "common/utility/serializedData.h"
#include "scriptRuntime/structDef.h"
#include "utility/enumNameMap.h"

using namespace BraneScript;

namespace TypeUtils
{
    template<typename T>
    void construct(byte* var)
    {
        new(var) T();
    }
    template<typename T>
    void deconstruct(byte* var)
    {
        ((T*)var)->~T();
    }
    template<typename T>
    void copy(byte* dest, const byte* source)
    {
        *((T*)dest) = *((T*)source);
    }
    template<typename T>
    void move(byte* dest, byte* source)
    {
        *((T*)dest) = std::move(*((T*)source));
    }
    template<typename T>
    void serialize(OutputSerializer& data, const byte* source)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Cannot serialize non-trivially copyable type");
        data << *(T*)(source);
    }
    template<typename T>
    void deserialize(InputSerializer& data, byte* source)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Cannot serialize non-trivially copyable type");
        data >> *(T*)(source);
    }


    void construct(const TypeDef* type, byte* var)
    {
        assert(type);
        switch (type->storageType)
        {
            case ValueType::Void:
                assert(false && "Cannot construct void type");
            case ValueType::Bool:
                construct<bool>(var);
                return;
            case ValueType::Char:
                construct<char>(var);
                return;
            case ValueType::Int32:
                construct<int32_t>(var);
                return;
            case ValueType::Int64:
                construct<int64_t>(var);
                return;
            case ValueType::UInt32:
                construct<uint32_t>(var);
                return;
            case ValueType::UInt64:
                construct<uint64_t>(var);
                return;
            case ValueType::Float32:
                construct<float>(var);
                return;
            case ValueType::Float64:
                construct<std::string>(var);
                return;
            case ValueType::FuncRef:
                construct<void(*)(void)>(var);
            case ValueType::Struct:
                auto* structDef = dynamic_cast<const BraneScript::StructDef*>(type);
                structDef->constructor(var);
                return;
        }
    }

    void deconstruct(const TypeDef* type, byte* var)
    {
        assert(type && type->storageType != ValueType::Void);
        if(auto* structDef = dynamic_cast<const BraneScript::StructDef*>(type))
            structDef->destructor(var);
    }

    void copy(const TypeDef* type, byte* dest, const byte* source)
    {
        assert(type);
        switch (type->storageType)
        {
            case ValueType::Void:
                assert(false && "Cannot construct void type");
            case ValueType::Bool:
                copy<bool>(dest, source);
                return;
            case ValueType::Char:
                copy<char>(dest, source);
                return;
            case ValueType::Int32:
                copy<int32_t>(dest, source);
                return;
            case ValueType::Int64:
                copy<int64_t>(dest, source);
                return;
            case ValueType::UInt32:
                copy<uint32_t>(dest, source);
                return;
            case ValueType::UInt64:
                copy<uint64_t>(dest, source);
                return;
            case ValueType::Float32:
                copy<float>(dest, source);
                return;
            case ValueType::Float64:
                copy<std::string>(dest, source);
                return;
            case ValueType::FuncRef:
                copy<void(*)(void)>(dest, source);
            case ValueType::Struct:
                auto* structDef = dynamic_cast<const BraneScript::StructDef*>(type);
                structDef->copyConstructor(dest, source);
                return;
        }
    }

    void move(const TypeDef* type, byte* dest, byte* source)
    {
        assert(type);
        switch (type->storageType)
        {
            case ValueType::Void:
                assert(false && "Cannot construct void type");
            case ValueType::Bool:
                move<bool>(dest, source);
                return;
            case ValueType::Char:
                move<char>(dest, source);
                return;
            case ValueType::Int32:
                move<int32_t>(dest, source);
                return;
            case ValueType::Int64:
                move<int64_t>(dest, source);
                return;
            case ValueType::UInt32:
                move<uint32_t>(dest, source);
                return;
            case ValueType::UInt64:
                move<uint64_t>(dest, source);
                return;
            case ValueType::Float32:
                move<float>(dest, source);
                return;
            case ValueType::Float64:
                move<std::string>(dest, source);
                return;
            case ValueType::FuncRef:
                move<void(*)(void)>(dest, source);
            case ValueType::Struct:
                auto* structDef = dynamic_cast<const BraneScript::StructDef*>(type);
                structDef->moveConstructor(dest, source);
                return;
        }
    }

    void serialize(const TypeDef* type, OutputSerializer data, const byte* source)
    {
        assert(type);
        switch (type->storageType)
        {
            case ValueType::Void:
                assert(false && "Cannot serialize void type");
                return;
            case ValueType::Bool:
                serialize<bool>(data, source);
                return;
            case ValueType::Char:
                serialize<char>(data, source);
                return;
            case ValueType::Int32:
                serialize<int32_t>(data, source);
                return;
            case ValueType::Int64:
                serialize<int64_t>(data, source);
                return;
            case ValueType::UInt32:
                serialize<uint32_t>(data, source);
                return;
            case ValueType::UInt64:
                serialize<uint64_t>(data, source);
                return;
            case ValueType::Float32:
                serialize<float>(data, source);
                return;
            case ValueType::Float64:
                serialize<double>(data, source);
                return;
            case ValueType::FuncRef:
                return;
            case ValueType::Struct:
            {
                auto* structDef = dynamic_cast<const BraneScript::StructDef*>(type);
                for(auto& m : structDef->memberVars)
                {
                    if(!m.type.isRef)
                        serialize(m.type.def, data, source + m.offset);
                    else
                    {
                        auto id = uint64_t(*((void**)(source + m.offset)));
                        serialize<uint64_t>(data, (byte*)&id);
                    }
                }
                return;
            }
            default:
                assert(false);
        }
    }

    void deserialize(const TypeDef* type, InputSerializer data, byte* source)
    {
        assert(type);
        switch (type->storageType)
        {
            case ValueType::Void:
                assert(false && "Cannot serialize void type");
                return;
            case ValueType::Bool:
                deserialize<bool>(data, source);
                return;
            case ValueType::Char:
                deserialize<char>(data, source);
                return;
            case ValueType::Int32:
                deserialize<int32_t>(data, source);
                return;
            case ValueType::Int64:
                deserialize<int64_t>(data, source);
                return;
            case ValueType::UInt32:
                deserialize<uint32_t>(data, source);
                return;
            case ValueType::UInt64:
                deserialize<uint64_t>(data, source);
                return;
            case ValueType::Float32:
                deserialize<float>(data, source);
                return;
            case ValueType::Float64:
                deserialize<double>(data, source);
                return;
            case ValueType::FuncRef:
                return;
            case ValueType::Struct:
            {
                auto* structDef = dynamic_cast<const BraneScript::StructDef*>(type);
                for(auto& m : structDef->memberVars)
                {
                    if(!m.type.isRef)
                        deserialize(m.type.def, data, source + m.offset);
                    else
                    {
                        uint64_t id = 0;
                        deserialize<uint64_t>(data, (byte*)&id);
                        *((void**)(source + m.offset)) = (void*)id;
                    }
                }
                return;
            }
            default:
                assert(false);
        }
    }
}