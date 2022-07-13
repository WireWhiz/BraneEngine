#pragma once
#include <assets/types/componentAsset.h>
#include "virtualType.h"
#include "structMembers.h"
#include <vector>
#include <string>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <iterator>
#include <set>
#include <utility/staticIndexVector.h>
#include <unordered_set>

#ifdef _64BIT
#define WORD_SIZE 8
#endif
#ifdef _32BIT
#define WORD_SIZE 4
#endif

class ComponentDescription
{
	struct Member{
		VirtualType::Type type;
		size_t offset;
	};

	std::vector<Member> _members;
	size_t _size;


	std::vector<size_t> generateOffsets(const std::vector<VirtualType::Type>&);
public:
	ComponentID id;
	std::string name;
	const ComponentAsset* asset;

	ComponentDescription(const ComponentAsset* asset);
	ComponentDescription(const std::vector<VirtualType::Type>& members);
	ComponentDescription(const std::vector<VirtualType::Type>& members, const std::vector<size_t>& offsets, size_t size);
	void construct(byte* component) const;
	void deconstruct(byte* component) const;
	void serialize(OSerializedData sData, byte* component) const;
	void deserialize(ISerializedData sData, byte* component) const;
	void copy(byte* src, byte* dest) const;
	void move(byte* src, byte* dest) const;
	const std::vector<Member>& members() const;
	size_t size() const;
};

class VirtualComponent
{
protected:
	byte* _data;
	const ComponentDescription* _description;
public:
	VirtualComponent(const VirtualComponent& source);
	VirtualComponent(VirtualComponent&& source);
	VirtualComponent(const ComponentDescription* definition);
	VirtualComponent(const ComponentDescription* definition, const byte* data);
	~VirtualComponent();
	VirtualComponent& operator=(const VirtualComponent& source);
	template<class T>
	T* getVar(size_t index) const
	{
		assert(index < _description->members().size());
		assert(_description->members()[index].offset + sizeof(T) <= _description->size());
		return getVirtual<T>(&_data[_description->members()[index].offset]);
	}
	template<class T>
	void setVar(size_t index, T value)
	{
		assert(index < _description->members().size());
		assert(_description->members()[index].offset + sizeof(T) <= _description->size());
		*(T*)&_data[_description->members()[index].offset] = value;
	}
	template<class T>
	T readVar(size_t index) const
	{
		assert(index < _description->members().size());
		assert(_description->members()[index].offset + sizeof(T) <= _description->size());
		return *(T*)&_data[_description->members()[index].offset];
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
		assert(index < _description->members().size());
		assert(_description->members()[index].offset + sizeof(T) <= _description->size());
		return getVirtual<T>(&_data[_description->members()[index].offset]);
	}
	template<class T>
	void setVar(size_t index, T value)
	{
		assert(index < _description->members().size());
		assert(_description->members()[index].offset + sizeof(T) <= _description->size());
		*(T*)&_data[_description->members()[index].offset] = value;
	}
	template<class T>
	T readVar(size_t index) const
	{
		assert(index < _description->members().size());
		assert(_description->members()[index].offset + sizeof(T) <= _description->size());
		return *(T*)&_data[_description->members()[index].offset];
	}
	byte* data() const;
	const ComponentDescription* description() const;
};

template <class T>
class NativeComponent
{
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
protected:
	typedef T ComponentType;
	static ComponentDescription* _description;
public:
	NativeComponent() = default;
	VirtualComponentView toVirtual()
	{
		return VirtualComponentView(_description, (byte*)this);
	}
	static T* fromVirtual(byte* data)
	{
		return (T*)data;
	}
	static const T* fromVirtual(const byte* data)
	{
		return (const T*)data;
	}
	static ComponentDescription* def()
	{
		return _description;
	}
};


template <class T> ComponentDescription* NativeComponent<T>::_description = nullptr;

class ComponentManager {
	staticIndexVector<std::unique_ptr<ComponentDescription>> _components;
	std::unordered_set<uint16_t> _externalComponents;
public:
	~ComponentManager();
	ComponentID createComponent(ComponentAsset* component);
	ComponentID createComponent(const std::vector<VirtualType::Type>& component, const std::string& name);
	ComponentID registerComponent(ComponentDescription* componentDescription);
	const ComponentDescription* getComponent(ComponentID id);
	void eraseComponent(ComponentID id);
};