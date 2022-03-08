#pragma once
#include <assets/types/componentAsset.h>
#include "virtualType.h"
#include "structMembers.h"
#include <vector>
#include <string>
#include <assert.h>
#include <stdlib.h>
#include <memory>
#include <mutex>
#include <iterator>
#include <set>


extern std::vector<ComponentAsset*> nativeComponentDefinitions;

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
public:
	static ComponentAsset* constructDef()
	{
		if(_def != nullptr)
			return _def;
		AssetID id;
		std::vector<VirtualType*> vars = T::getMembers(id);
		_def = new ComponentAsset(vars, id, sizeof(T));
		std::string name = typeid(T).name();
		size_t nameStart = name.find_last_of("::") + 1;
		if(nameStart == std::string::npos)
			nameStart = 0;
		_def->name =  name.substr(nameStart, name.size() - nameStart);
		return _def;
	}
protected:
	typedef T ComponentType;
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
		return _def;
	}
};
template <class T> ComponentAsset* NativeComponent<T>::_def = nullptr;

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
