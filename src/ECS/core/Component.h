#pragma once
#include "VirtualType.h"
#include <vector>
#include <string>
#include <assert.h>


typedef uint64_t ComponentID;
const ComponentID nullComponent = 0;

struct NativeVarDef
{
	VirtualType type;
	size_t index;
	NativeVarDef(size_t index, VirtualType type)
	{
		this->type = type;
		this->index = index;
	}
};

class ComponentDefinition
{
private:
	size_t _numTypes;
	size_t _size;
	bool _initalized;
	VirtualType* _types;
	size_t* _byteIndices;
	ComponentID _id;
public:
	ComponentDefinition();
	ComponentDefinition(const ComponentDefinition& source);
	ComponentDefinition(size_t _numTypes, ComponentID id);
	~ComponentDefinition();
	void setIndexType(size_t index, VirtualType type);
	void initalize();
	void initalize(const std::vector<NativeVarDef> vars, size_t size);
	ComponentID id() const;
	size_t size() const;
	size_t getByteIndex(size_t index) const;
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
	void constructDef()
	{
		if (_def == nullptr)
		{
			std::vector<NativeVarDef> vars;
			this->getVariables(vars);

			assert(vars.size() > 0);
			_def = new ComponentDefinition(vars.size(), id);
			_def->initalize(vars, sizeof(T));
		}
	}
protected:
	static ComponentDefinition* _def;
	virtual void getVariables(std::vector<NativeVarDef>& variables) = 0;
	size_t getVarIndex(void* var)
	{
		size_t index = (size_t)var - (size_t)this;;
		assert(0 <= index < sizeof(T));
		return index;
	}
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
		constructDef();
		return VirtualComponentPtr(_def, (byte*)this);
	}
	ComponentDefinition* def()
	{
		return _def;
	}
};
template <class T, size_t id> ComponentDefinition* NativeComponent<T, id>::_def = nullptr;

class VirtualComponentVector
{
private:
	std::vector<byte> _data;
	ComponentDefinition* _def;
public:
	VirtualComponentVector(ComponentDefinition* definition, size_t initalSize);
	VirtualComponentVector(ComponentDefinition* definition);
	byte* getComponentData(size_t index) const;
	VirtualComponentPtr getComponent(size_t index) const;
	template<class T>
	const T readComponentVar(size_t structIndex, size_t varIndex) const
	{
		structIndex = byteIndex(structIndex);
		varIndex = _def->getByteIndex(varIndex);
		return readVirtual<T>(&_data[structIndex + varIndex]);
	}
	template<class T>
	void setComponentVar(size_t structIndex, size_t varIndex, T value)
	{
		structIndex = byteIndex(structIndex);
		varIndex = _def->getByteIndex(varIndex);
		setVirtual(&_data[structIndex + varIndex], value);
	}
	size_t byteIndex(size_t structIndex) const;
	size_t size() const;
	ComponentDefinition* def() const;
	void reserve(size_t size);
	void pushBack(VirtualComponent& virtualComponent);
	void pushBack(VirtualComponentPtr& virtualComponentPtr);
	void pushEmpty();
	void swapRemove(size_t index);
	void copy(const VirtualComponentVector& source, size_t sourceIndex, size_t destIndex);
};

