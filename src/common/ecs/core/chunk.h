#pragma once
#include "archetype.h"

class Chunk
{
	const size_t _size;
	size_t _maxCapacity;
	std::vector<byte> _data;
	VirtualArchetype* _archetype;
	std::vector<size_t> _componentIndices;
public:
	VirtualComponentChunk(size_t size);
	void setDef(VirtualArchetype* archetype);
};