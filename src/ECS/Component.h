#pragma once
#include "VirtualType.h"
#include <vector>
#include <string>
#include <assert.h>


typedef uint64_t ComponentID;

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
	std::string identifier;
	ComponentDefinition();
	ComponentDefinition(const ComponentDefinition& source);
	ComponentDefinition(size_t _numTypes, ComponentID id);
	~ComponentDefinition();
	void setIndexType(size_t index, VirtualType type);
	void initalize();
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
	void pushBack(VirtualComponent& virtualStruct);
	void pushBack();
	void swapRemove(size_t index);
	void copy(const VirtualComponentVector& source, size_t sourceIndex, size_t destIndex);
};

