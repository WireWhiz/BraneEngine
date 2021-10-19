#pragma once
#include <vector>
#include "virtualType.h"
#include "archetype.h"

class Chunk
{
	const size_t _size;
	size_t _maxCapacity;
	std::vector<byte> _data;
	Archetype* _archetype;
	std::vector<size_t> _componentIndices;
public:
	Chunk(size_t size);
	void setDef(Archetype* archetype);
};