#pragma once
#include <cstdlib>


typedef unsigned char byte;

enum VirtualType
{
	virtualNone = 0,
	virtualBool = 1,
	virtualInt = 2,
	virtualFloat = 3,
};

constexpr size_t sizeofVirtual(VirtualType vt)
{
	size_t sizes[4] = {
		0,
		sizeof(bool),
		sizeof(int),
		sizeof(float)
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