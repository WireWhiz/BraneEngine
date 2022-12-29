#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <iterator>
#include <set>
#include "common/utility/staticIndexVector.h"
#include <unordered_set>
#include "functionHandle.h"
#include "structDefinition.h"

namespace BraneScript
{
    class StructDef;
    class Linker;
}

class ComponentAsset;
using ComponentID = uint32_t;
using byte = uint8_t;

class ComponentDescription
{
    const BraneScript::StructDef* _type;
    BraneScript::FunctionHandle<void, byte*> _constructor;
    BraneScript::FunctionHandle<void, byte*> _destructor;
    BraneScript::FunctionHandle<void, byte*, byte*> _move;
    BraneScript::FunctionHandle<void, byte*, const byte*> _copy;
public:
    ComponentID id;

    ComponentDescription(const BraneScript::StructDef* type, const BraneScript::Linker* linker);
    void construct(byte* component) const;
    void deconstruct(byte* component) const;
    void serialize(OutputSerializer& sData, byte* component) const;
    void deserialize(InputSerializer& sData, byte* component) const;
    void copy(const byte* src, byte* dest) const;
    void move(byte* src, byte* dest) const;

    const BraneScript::StructDef* type() const;
    const std::string& name() const;
    size_t size() const;
};

class VirtualComponentView;

class VirtualComponent
{
protected:
    byte* _data;
    const ComponentDescription* _description;
public:
    VirtualComponent(const VirtualComponent& source);
    VirtualComponent(const VirtualComponentView& source);
    VirtualComponent(VirtualComponent&& source);
    VirtualComponent(const ComponentDescription* definition);
    VirtualComponent(const ComponentDescription* definition, const byte* data);
    ~VirtualComponent();
    VirtualComponent& operator=(const VirtualComponent& source);
    VirtualComponent& operator=(const VirtualComponentView& source);
    template<class T>
    T* getVar(size_t index) const
    {
        assert(index < _description->type()->memberVars().size());
        assert(_description->type()->memberVars()[index].offset + sizeof(T) <= _description->size());
        return getVirtual<T>(&_data[_description->type()->memberVars()[index].offset]);
    }
    template<class T>
    void setVar(size_t index, T value)
    {
        assert(index < _description->type()->memberVars().size());
        assert(_description->type()->memberVars()[index].offset + sizeof(T) <= _description->size());
        *(T*)&_data[_description->type()->memberVars()[index].offset] = value;
    }
    template<class T>
    T readVar(size_t index) const
    {
        assert(index < _description->type()->memberVars().size());
        assert(_description->type()->memberVars()[index].offset + sizeof(T) <= _description->size());
        return *(T*)&_data[_description->type()->memberVars()[index].offset];
    }
    byte* data() const;
    const ComponentDescription* description() const;
};

class VirtualComponentView
{
protected:
    byte* _data;
    const ComponentDescription* _description;
public:
    VirtualComponentView(const VirtualComponent& source);
    VirtualComponentView(const ComponentDescription* description, byte* data);
    template<class T>
    T* getVar(size_t index) const
    {
        assert(index < _description->type()->memberVars().size());
        assert(_description->type()->memberVars()[index].offset + sizeof(T) <= _description->size());
        return getVirtual<T>(&_data[_description->type()->memberVars()[index].offset]);
    }
    template<class T>
    void setVar(size_t index, T value)
    {
        assert(index < _description->type()->memberVars().size());
        assert(_description->type()->memberVars()[index].offset + sizeof(T) <= _description->size());
        *(T*)&_data[_description->type()->memberVars()[index].offset] = value;
    }
    template<class T>
    T readVar(size_t index) const
    {
        assert(index < _description->type()->memberVars().size());
        assert(_description->type()->memberVars()[index].offset + sizeof(T) <= _description->size());
        return *(T*)&_data[_description->type()->memberVars()[index].offset];
    }
    byte* data() const;
    const ComponentDescription* description() const;
};

