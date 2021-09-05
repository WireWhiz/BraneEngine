#pragma once
#include <cstdlib>
#include <glm/glm.hpp>

typedef unsigned char byte;

enum VirtualType
{
	virtualNone = 0,
	virtualAddress,
	virtualBool,
	virtualInt,
	virtualFloat,
	virtualFloat2,
	virtualFloat3,
	virtualFloat4,
	virtualFloat4x4
};

constexpr size_t sizeofVirtual(VirtualType vt)
{
	size_t sizes[] = {
		0,
		sizeof(uint64_t),
		sizeof(bool),
		sizeof(int),
		sizeof(float),
		sizeof(glm::vec2),
		sizeof(glm::vec3),
		sizeof(glm::vec4),
		sizeof(glm::mat4x4)
	};
	return sizes[(byte)vt];
};

template <class T>
constexpr inline
T* getVirtual(byte* var)
{
	return (T*)var;
}

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
void setVirtual(byte* var, T value)
{
	*(T*)var = value;
}