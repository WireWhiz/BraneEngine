#pragma once
#include "VirtualType.h"
#include <vector>
#include <string>
#include <assert.h>
#include <stdlib.h>
#include <memory>
#include <mutex>

typedef uint64_t ComponentID;
const ComponentID nullComponent = 0;

class ComponentDefinition
{
private:
	ComponentID _id;
	size_t _size;
	std::vector<std::shared_ptr<VirtualType>> _types;
public:
	ComponentDefinition(const ComponentDefinition& source);
	ComponentDefinition(std::vector<std::shared_ptr<VirtualType>>& types , ComponentID id);
	~ComponentDefinition();
	void setSize(size_t size);
	ComponentID id() const;
	size_t size() const;
	size_t getByteIndex(size_t index) const;
	const std::vector<std::shared_ptr<VirtualType>>& types();
};

class VirtualComponent
{
protected:
	byte* _data;
	ComponentDefinition* _def;
public:
	VirtualComponent(const VirtualComponent& source);
	VirtualComponent(ComponentDefinition* definition);
	VirtualComponent(ComponentDefinition* definition, byte* data);
	~VirtualComponent();
	template<class T>
	T* getVar(size_t index) const
	{
		return getVirtual<T>(&_data[_def->getByteIndex(index)]);
	}
	template<class T>
	void setVar(size_t index, T value)
	{
		*(T*)&_data[_def->getByteIndex(index)] = value;
	}
	template<class T>
	T readVar(size_t index) const
	{
		return *(T*)&_data[_def->getByteIndex(index)];
	}
	byte* data() const;
	ComponentDefinition* def() const;
};

class VirtualComponentPtr
{
	byte* _data;
	ComponentDefinition* _def;
public:
	VirtualComponentPtr(ComponentDefinition* definition, byte* data);
	template<class T>
	T* getVar(size_t index) const
	{
		return getVirtual<T>(&_data[_def->getByteIndex(index)]);
	}
	template<class T>
	void setVar(size_t index, T value)
	{
		setVirtual<T>(&_data[_def->getByteIndex(index)], value);
	}
	template<class T>
	T readVar(size_t index) const
	{
		return readVirtual<T>(&_data[_def->getByteIndex(index)]);
	}
	ComponentDefinition* def() const;
	byte* data() const;
};

template <class T, size_t id>
class NativeComponent
{
private:
	static void constructDef()
	{
		std::vector<std::shared_ptr<VirtualType>> vars;
		T o;
		o.getVariableTypes(vars);
		_def = new ComponentDefinition(vars, id);
		_def->setSize(sizeof(T));
	}
protected:
	static std::once_flag _flag;
	static ComponentDefinition* _def;
	// This virtual is actually never used, keeping it for reference
	virtual void getVariableTypes(std::vector<std::shared_ptr<VirtualType>>& types) = 0;
public:
	NativeComponent()
	{
		
	}
	static T* fromVirtual(byte* data)
	{
		return (T*)data;
	}
	VirtualComponentPtr toVirtual()
	{
		return VirtualComponentPtr(def(), (byte*)this);
	}
	static ComponentDefinition* def()
	{
		std::call_once(_flag, constructDef);
		return _def;
	}
};
//This is a memory leak, but as we want to keep the def around for the enitre lifetime of the program it doesn't matter
template <class T, size_t id> ComponentDefinition* NativeComponent<T, id>::_def;
template <class T, size_t id> std::once_flag NativeComponent<T, id>::_flag;

class VirtualComponentVector
{
private:
	std::vector<byte> _data;
	ComponentDefinition* _def;
public:
	VirtualComponentVector(ComponentDefinition* definition, size_t initalSize);
	VirtualComponentVector(ComponentDefinition* definition);
	VirtualComponentVector(const VirtualComponentVector& other);
	byte* getComponentData(size_t index) const;
	VirtualComponentPtr getComponent(size_t index) const;
	template<class T>
	const T readComponentVar(size_t componentIndex, size_t varIndex) const
	{
		componentIndex = structIndex(componentIndex);
		varIndex = _def->getByteIndex(varIndex);
		return readVirtual<T>(&_data[componentIndex + varIndex]);
	}
	template<class T>
	void setComponentVar(size_t componentIndex, size_t varIndex, T value)
	{
		componentIndex = structIndex(componentIndex);
		varIndex = _def->getByteIndex(varIndex);
		setVirtual(&_data[componentIndex + varIndex], value);
	}
	size_t structIndex(size_t index) const;
	size_t size() const;
	ComponentDefinition* def() const;
	void reserve(size_t size);
	void pushBack(VirtualComponent& virtualComponent);
	void pushBack(VirtualComponentPtr& virtualComponentPtr);
	void pushEmpty();
	void swapRemove(size_t index);
	void copy(const VirtualComponentVector* source, size_t sourceIndex, size_t destIndex);
};

