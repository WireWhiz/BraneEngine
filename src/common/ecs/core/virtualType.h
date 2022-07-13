#pragma once
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <byte.h>
#include <utility/serializedData.h>
#include <utility/inlineArray.h>

#ifndef EntityID
typedef uint32_t EntityID;
#endif

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

namespace VirtualType
{
	enum Type{
		virtualUnknown = 0,
		virtualBool,
		virtualInt,
		virtualInt64,
		virtualUInt,
		virtualUInt64,
		virtualFloat,
		virtualString,
		virtualAssetID,
		virtualVec3,
		virtualVec4,
		virtualQuat,
		virtualMat4,
		virtualFloatArray,
		virtualIntArray,
		virtualUIntArray
	};
	template<typename T>
	Type type();
	std::string typeToString(Type type);
	Type stringToType(const std::string& type);
	void serialize(Type type, OSerializedData& data, const byte* source);
	void deserialize(Type type, ISerializedData& data, byte* source);
	size_t size(Type type);
	void construct(Type type, byte* var);
	void deconstruct(Type type, byte* var);
	void copy(Type type, byte* dest, const byte* source);
	void move(Type type, byte* dest, byte* source);
	template<typename T>
	void serialize(OSerializedData& data, const byte* source)
	{
		data << *getVirtual<T>(source);
	}
	template<typename T>
	void deserialize(ISerializedData& data, byte* source)
	{
		data >> *getVirtual<T>(source);
	}
	template<typename T>
	void construct(byte* var)
	{
		new(var) T();
	}
	template<typename T>
	void deconstruct(byte* var)
	{
		((T*)var)->~T();
	}
	template<typename T>
	void copy(byte* dest, const byte* source)
	{
		*((T*)dest) = *((T*)source);
	}
	template<typename T>
	void move(byte* dest, byte* source)
	{
		*((T*)dest) = std::move(*((T*)source));
	}
};

template<typename T>
VirtualType::Type VirtualType::type()
{
	if constexpr(std::is_same<T, bool>().value)
		return Type::virtualBool;
	if constexpr(std::is_same<T, int32_t>().value)
		return Type::virtualInt;
	if constexpr(std::is_same<T, uint32_t>().value)
		return Type::virtualUInt;
	if constexpr(std::is_same<T, int64_t>().value)
		return Type::virtualInt64;
	if constexpr(std::is_same<T, uint64_t>().value)
		return Type::virtualUInt64;
	if constexpr(std::is_same<T, float>().value)
		return Type::virtualFloat;
	if constexpr(std::is_same<T, std::string>().value)
		return Type::virtualString;
	if constexpr(std::is_same<T, AssetID>().value)
		return Type::virtualAssetID;
	if constexpr(std::is_same<T, glm::vec3>().value)
		return Type::virtualVec3;
	if constexpr(std::is_same<T, glm::vec4>().value)
		return Type::virtualVec4;
	if constexpr(std::is_same<T, glm::quat>().value)
		return Type::virtualQuat;
	if constexpr(std::is_same<T, glm::mat4>().value)
		return Type::virtualMat4;
	if constexpr(std::is_same<T, inlineFloatArray>().value)
		return Type::virtualFloatArray;
	if constexpr(std::is_same<T, inlineIntArray>().value)
		return Type::virtualIntArray;
	if constexpr(std::is_same<T, inlineUIntArray>().value)
		return Type::virtualUIntArray;

	std::cerr << "Tried to find type of: [" << typeid(T).name() << "] and failed"<< std::endl;
	assert(false);
	return Type::virtualUnknown;
}

