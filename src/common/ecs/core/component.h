#pragma once
#include "virtualType.h"
#include <vector>
#include <string>
#include <assert.h>
#include <stdlib.h>
#include <memory>
#include <mutex>
#include <iterator>
#include <assets/types/componentAsset.h>

// Class that always has components sorted
class ComponentSet
{
private:
	std::vector<const ComponentAsset*> _components;
public:
	void add(const ComponentAsset* component);
	void remove(const  ComponentAsset* component);
	const std::vector<const ComponentAsset*>& components() const;

	bool contains(const ComponentAsset* component) const;
	bool contains(const ComponentSet& subset) const;
	size_t index(const ComponentAsset* component) const;
	void indicies(const ComponentSet& subset, size_t* indices) const;

	size_t size() const;

	const ComponentAsset* operator[](size_t index) const;
	typename std::vector<const ComponentAsset*>::const_iterator begin() const;
	typename std::vector<const ComponentAsset*>::const_iterator end() const;
};

class VirtualComponent
{
protected:
	byte* _data;
	const ComponentAsset* _def;
public:
	VirtualComponent(const VirtualComponent& source);
	VirtualComponent(VirtualComponent&& source);
	VirtualComponent(const ComponentAsset* definition);
	VirtualComponent(const ComponentAsset* definition, const byte* data);
	~VirtualComponent();
	VirtualComponent& operator=(const VirtualComponent& source);
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
	const ComponentAsset* def() const;
};

class VirtualComponentPtr
{
	byte* _data;
	const ComponentAsset* _def;
public:
	VirtualComponentPtr(const ComponentAsset* definition, byte* const data);
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
	const ComponentAsset* def() const;
	byte* data() const;
};

template <class T>
class NativeComponent
{
private:
	static void constructDef()
	{
		std::vector<std::unique_ptr<VirtualType>> vars;
		AssetID id;
		T o;
		o.getComponentData(vars, id);
		_def = new ComponentAsset(vars, id);
		_def->setSize(sizeof(T));
	}
protected:
	static std::once_flag _flag;
	static ComponentAsset* _def;
public:
	NativeComponent() = default;
	static T* fromVirtual(byte* data)
	{
		return (T*)data;
	}
	static const T* fromVirtual(const byte* data)
	{
		return (const T*)data;
	}
	VirtualComponentPtr toVirtual()
	{
		return VirtualComponentPtr(def(), (byte*)this);
	}
	static ComponentAsset* def()
	{
		std::call_once(_flag, constructDef);
		return _def;
	}
};
template <class T> ComponentAsset* NativeComponent<T>::_def;
template <class T> std::once_flag NativeComponent<T>::_flag;

class VirtualComponentVector
{
private:
	std::vector<byte> _data;
	const ComponentAsset* _def;
public:
	VirtualComponentVector(const ComponentAsset* definition, size_t initalSize);
	VirtualComponentVector(const ComponentAsset* definition);
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
	const ComponentAsset* def() const;
	void reserve(size_t size);
	void pushBack(VirtualComponent& virtualComponent);
	void pushBack(VirtualComponentPtr& virtualComponentPtr);
	void pushEmpty();
	void remove(size_t index);
	void copy(const VirtualComponentVector* source, size_t sourceIndex, size_t destIndex);
};
