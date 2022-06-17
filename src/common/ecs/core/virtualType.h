#pragma once
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <byte.h>
#include <utility/serializedData.h>
#include <json/json.h>

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

class VirtualType
{
public:
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
		virtualMat4,
		virtualFloatVector,
		virtualIntVector,
		virtualUIntVector
	};
public:
	template<typename T>
	static Type type();
	static std::string typeToString(Type type);
	static Type stringToType(const std::string& type);
	static void serialize(Type type, OSerializedData& data, const byte* source);
	static void deserialize(Type type, ISerializedData& data, byte* source);
	static size_t size(Type type);
	static void construct(Type type, byte* var);
	static void deconstruct(Type type, byte* var);
	static void copy(Type type, byte* dest, const byte* source);
	static void move(Type type, byte* dest, const byte* source);
	template<typename T>
	static void serialize(OSerializedData& data, const byte* source)
	{
		data << *getVirtual<T>(source);
	}
	template<typename T>
	static void deserialize(ISerializedData& data, byte* source)
	{
		data >> *getVirtual<T>(source);
	}
	template<typename T>
	static void construct(byte* var)
	{
		new(var) T();
	}
	template<typename T>
	static void deconstruct(byte* var)
	{
		((T*)var)->~T();
	}
	template<typename T>
	static void copy(byte* dest, const byte* source)
	{
		*((T*)dest) = *((T*)source);
	}
	template<typename T>
	static void move(byte* dest, const byte* source)
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
	if constexpr(std::is_same<T, glm::mat4>().value)
		return Type::virtualMat4;
	if constexpr(std::is_same<T, std::vector<float>>().value)
		return Type::virtualFloatVector;
	if constexpr(std::is_same<T, std::vector<int32_t>>().value)
		return Type::virtualIntVector;
	if constexpr(std::is_same<T, std::vector<uint32_t>>().value)
		return Type::virtualUIntVector;

	std::cerr << "Tried to find type of: [" << typeid(T).name() << "] and failed"<< std::endl;
	return Type::virtualUnknown;
}

