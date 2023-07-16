#pragma once

#include <cassert>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include "common/utility/staticIndexVector.h"
#include "scriptRuntime/funcRef.h"
#include "scriptRuntime/nativeLibrary.h"
#include "scriptRuntime/structDef.h"
#include "utility/typeUtils.h"
#include "utility/serializedData.h"
#include <unordered_set>

namespace BraneScript
{
    class StructDef;
}

class ComponentAsset;
using ComponentID = uint32_t;
using byte = uint8_t;

class ComponentDescription
{
    const BraneScript::StructDef* _type;
public:
    ComponentID id;

    ComponentDescription(const BraneScript::StructDef* type);
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
        assert(index < _description->type()->memberVars.size());
        assert(_description->type()->memberVars[index].offset + sizeof(T) <= _description->size());
        return getVirtual<T>(&_data[_description->type()->memberVars[index].offset]);
    }
    template<class T>
    void setVar(size_t index, T value)
    {
        assert(index < _description->type()->memberVars.size());
        assert(_description->type()->memberVars[index].offset + sizeof(T) <= _description->size());
        *(T*)&_data[_description->type()->memberVars[index].offset] = value;
    }
    template<class T>
    T readVar(size_t index) const
    {
        assert(index < _description->type()->memberVars.size());
        assert(_description->type()->memberVars[index].offset + sizeof(T) <= _description->size());
        return *(T*)&_data[_description->type()->memberVars[index].offset];
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
        assert(index < _description->type()->memberVars.size());
        assert(_description->type()->memberVars[index].offset + sizeof(T) <= _description->size());
        return getVirtual<T>(&_data[_description->type()->memberVars[index].offset]);
    }
    template<class T>
    void setVar(size_t index, T value)
    {
        assert(index < _description->type()->memberVars.size());
        assert(_description->type()->memberVars[index].offset + sizeof(T) <= _description->size());
        *(T*)&_data[_description->type()->memberVars[index].offset] = value;
    }
    template<class T>
    T readVar(size_t index) const
    {
        assert(index < _description->type()->memberVars.size());
        assert(_description->type()->memberVars[index].offset + sizeof(T) <= _description->size());
        return *(T*)&_data[_description->type()->memberVars[index].offset];
    }
    byte* data() const;
    const ComponentDescription* description() const;
};

