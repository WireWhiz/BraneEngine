#include <cstdlib>

typedef unsigned char byte;

enum VirtualType
{
	virtualNone = 0,
	virtualBool = 1,
	virtualInt = 2,
	virtualFloat = 3,
};

constexpr size_t sizeofVirtual(VirtualType vt);

template <class T>
constexpr inline
T* readVirtual(byte* var)
{
	return (T*)var;
}