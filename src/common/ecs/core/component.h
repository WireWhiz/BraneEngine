#pragma once
#include "VirtualType.h"
#include <vector>
#include <string>
#include <assert.h>
#include <stdlib.h>
#include <memory>
#include <mutex>
#include <iterator>

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
	ComponentDefinition(const std::vector<std::shared_ptr<VirtualType>>& types , ComponentID id);
	~ComponentDefinition();
	void setSize(size_t size);
	ComponentID id() const;
	size_t size() const;
	size_t getByteIndex(size_t index) const;
	const std::vector<std::shared_ptr<VirtualType>>& types() const;
};

// Class that always has components sorted
class ComponentSet
{
private:
	std::vector<const  ComponentDefinition*> _components;
public:
	void add(const ComponentDefinition* component);
	void remove(const  ComponentDefinition* component);
	const std::vector<const ComponentDefinition*>& components() const;

	bool contains(const ComponentDefinition* component) const;
	bool contains(const ComponentSet& subset) const;
	size_t index(const ComponentDefinition* component) const;
	void indicies(const ComponentSet& subset, size_t* indices) const;

	size_t size() const;

	const ComponentDefinition* operator[](size_t index) const;
	typename std::vector<const ComponentDefinition*>::const_iterator begin() const;
	typename std::vector<const ComponentDefinition*>::const_iterator end() const;
};

class VirtualComponent
{
protected:
	byte* _data;
	const ComponentDefinition* _def;
public:
	VirtualComponent(const VirtualComponent& source);
	VirtualComponent(const ComponentDefinition* definition);
	VirtualComponent(const ComponentDefinition* definition, const byte* data);
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
	const ComponentDefinition* def() const;
};

class VirtualComponentPtr
{
	byte* _data;
	const ComponentDefinition* _def;
public:
	VirtualComponentPtr(const ComponentDefinition* definition, byte* const data);
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
	const ComponentDefinition* def() const;
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
public:
	// This virtual is actually never used, keeping it for reference
	virtual void getVariableTypes(std::vector<std::shared_ptr<VirtualType>>& types) = 0;
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
template <class T, size_t id> ComponentDefinition* NativeComponent<T, id>::_def;
template <class T, size_t id> std::once_flag NativeComponent<T, id>::_flag;

class VirtualComponentVector
{
private:
	std::vector<byte> _data;
	const ComponentDefinition* _def;
public:
	VirtualComponentVector(const ComponentDefinition* definition, size_t initalSize);
	VirtualComponentVector(const ComponentDefinition* definition);
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
	const ComponentDefinition* def() const;
	void reserve(size_t size);
	void pushBack(VirtualComponent& virtualComponent);
	void pushBack(VirtualComponentPtr& virtualComponentPtr);
	void pushEmpty();
	void remove(size_t index);
	void copy(const VirtualComponentVector* source, size_t sourceIndex, size_t destIndex);
};
