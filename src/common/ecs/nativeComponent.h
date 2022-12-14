//
// Created by eli on 7/17/2022.
//

#ifndef BRANEENGINE_NATIVECOMPONENT_H
#define BRANEENGINE_NATIVECOMPONENT_H

#include "component.h"
#include "structMembers.h"
#include "structDefinition.h"
#include "nativeTypes.h"

template <class T>
class NativeComponent
{
protected:
    using ComponentType = T;
    static ComponentDescription* _description;
public:
    static ComponentDescription* constructDescription()
    {
        if(_description != nullptr)
            return _description;
        AssetID id;
        std::vector<VirtualType::Type> members = T::getMemberTypes();
        std::vector<size_t> offsets = T::getMemberOffsets();
        _description = new ComponentDescription(members, offsets, sizeof(T));
        _description->name =  T::getComponentName();
        return _description;
    }

    static BraneScript::StructDef* newTypeDef()
    {
        auto def = new BraneScript::StructDef(T::getComponentName());
        auto names = T::getMemberNames();
        auto offsets = T::getMemberOffsets();
        auto types = T::getMemberTypes();
        for(size_t i = 0; i < names.size(); ++i)
        {
            BraneScript::TypeDef* type = nullptr;
            switch(types[i])
            {
                {
                    case VirtualType::virtualBool:
                        type = BraneScript::getNativeTypeDef(BraneScript::ValueType::Bool);
                    break;
                    case VirtualType::virtualInt:
                        type = BraneScript::getNativeTypeDef(BraneScript::ValueType::Int32);
                    break;
                    case VirtualType::virtualInt64:
                        type = BraneScript::getNativeTypeDef(BraneScript::ValueType::Int64);
                    break;
                    case VirtualType::virtualFloat:
                        type = BraneScript::getNativeTypeDef(BraneScript::ValueType::Float32);
                    break;
                    default:
                        assert(false);
                        break;
                }
            }
            if(type)
                def->addMemberVar(names[i], type, offsets[i]);
        }
        return def;
    }

    NativeComponent() = default;
    VirtualComponentView toVirtual() const
    {
        return VirtualComponentView(_description, (byte*)this);
    }
    operator VirtualComponentView() const
    {
        return toVirtual();
    }
    static T* fromVirtual(byte* data)
    {
        return (T*)data;
    }
    static const T* fromVirtual(const byte* data)
    {
        return (const T*)data;
    }
    static T* fromVirtual(VirtualComponentView component)
    {
        return (T*)component.data();
    }
    static ComponentDescription* def()
    {
        return _description;
    }
};




template <class T> ComponentDescription* NativeComponent<T>::_description = nullptr;

#endif //BRANEENGINE_NATIVECOMPONENT_H
