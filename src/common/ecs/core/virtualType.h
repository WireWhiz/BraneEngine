#pragma once
#include <cstdlib>
#include <glm/glm.hpp>
#include <byte.h>
#include <networking/serializedData.h>

template <class T>
constexpr inline
T* getVirtual(const byte* var)
{
	return (T*)var;
}

template <class T>
constexpr inline
T readVirtual(byte* var)
{
	return *(T*)var;
}

template <class T>
constexpr inline
T readVirtual(const byte* var)
{
	return *(T*)var;
}

template <class T>
constexpr inline
void setVirtual(const byte* var, T value)
{
	*(T*)var = value;
}

class VirtualComponentPtr;

class VirtualType
{
public:
	enum Type{
		virtualUnknown = 0,
		virtualBool,
		virtualInt,
		virtualUInt,
		virtualFloat,
		virtualString
	};
protected:
	size_t _offset;
public:
	VirtualType(size_t offset);
	void setOffset(size_t offset);
	size_t offset();
	virtual Type getType() = 0;
	static VirtualType* constructTypeOf(Type type);
	virtual void serialize(OSerializedData& data, const byte* source)  = 0;
	virtual void deserialize(ISerializedData& data, byte* source) = 0;
	virtual const size_t size() const = 0;
	virtual void construct(byte*) = 0;
	void construct(VirtualComponentPtr& vcp);
	virtual void copy(byte* dest, const byte* source) = 0;
	virtual void move(byte* dest, const byte* source) = 0;
	virtual void deconstruct(byte*) = 0;
	void deconstruct(VirtualComponentPtr& vcp);
};

template<typename T>
class VirtualVariable : public VirtualType
{
public:
	VirtualVariable() : VirtualType(0)
	{
	}
	VirtualVariable(size_t offset) : VirtualType(offset)
	{
	}

	Type getType() override //constexpr makes it so there are no branches here at runtime
	{
		if constexpr(std::is_same<T, bool>().value)
			return Type::virtualBool;
		if constexpr(std::is_same<T, int>().value)
			return Type::virtualInt;
		if constexpr(std::is_same<T, uint32_t>().value)
			return Type::virtualUInt;
		if constexpr(std::is_same<T, float>().value)
			return Type::virtualFloat;
		if constexpr(std::is_same<T, std::string>().value)
			return Type::virtualString;
		return Type::virtualUnknown;
	}

	void serialize(OSerializedData& data, const byte* source) override
	{
		data << *getVirtual<T>(source);
	}
	void deserialize(ISerializedData& data, byte* source) override
	{
		data >> *getVirtual<T>(source);
	}
	void construct(byte* var) override
	{
		new(var) T();
	}
	template<class... Params>
	void construct(byte* var, Params... params)
	{
		new(var) T(params...);
	}
	virtual void copy(byte* dest, const byte* source)
	{
		*((T*)dest) = *((T*)source);
	}
	virtual void move(byte* dest, const byte* source)
	{
		*((T*)dest) = std::move(*((T*)source));
	}
	void deconstruct(byte* var) override
	{
		((T*)var)->~T();
	}
	T* get(const byte* var)
	{
		return (T*)var;
	}
	const size_t size() const override
	{
		return sizeof(T);
	}
};

typedef VirtualVariable<bool>    VirtualBool;
typedef VirtualVariable<int64_t> VirtualInt;
typedef VirtualVariable<float>   VirtualFloat;