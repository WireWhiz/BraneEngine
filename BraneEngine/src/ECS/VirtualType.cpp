#include "VirtualType.h"

constexpr size_t sizeofVirtual(VirtualType vt)
{
	size_t sizes[3] = {
		sizeof(bool),
		sizeof(int),
		sizeof(float)
	};
	return sizes[(byte)vt];
}