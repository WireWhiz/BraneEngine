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
	NativeVarDef()
	{
		type = virtualNone;
		index = 0;
	}
	NativeVarDef(size_t index, VirtualType type)
	{
		this->type = type;
		this->index = index;
	}
};

class ComponentDefinition
{
private:
	ComponentID _id;
	size_t _numTypes;
	size_t _size;
	bool _initalized;
	VirtualType* _types;
	size_t* _byteIndices;
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
	static ComponentDefinition* constructDef()
	{
		std::vector<NativeVarDef> vars;
		T c;
		c.getVariableIndicies(vars);

		assert(vars.size() > 0);
		ComponentDefinition* def = new ComponentDefinition(vars.size(), id);
		def->initalize(vars, sizeof(T));
		return def;
	}
protected:
	static ComponentDefinition* _def;
	// This virtual is actually never used, keeping it for reference
	virtual void getVariableIndicies(std::vector<NativeVarDef>& variables) = 0;
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
		return VirtualComponentPtr(_def, (byte*)this);
	}
	static ComponentDefinition* def()
	{
		return _def;
	}
};
//This is a memory leak, but as we want to keep the def around for the enitre lifetime of the program it doesn't matter
template <class T, size_t id> ComponentDefinition* NativeComponent<T, id>::_def = NativeComponent<T, id>::constructDef();

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
	void copy(const VirtualComponentVector* source, size_t sourceIndex, size_t destIndex);
};

