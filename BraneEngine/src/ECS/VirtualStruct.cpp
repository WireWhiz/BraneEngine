#include "VirtualStruct.h"

VirtualStruct::VirtualStruct(VirtualStructDefinition* definition)
{
	def = definition;
	data = new byte[definition->size()];
}

VirtualStruct::~VirtualStruct()
{
	delete[] data;
}

VirtualStructDefinition::VirtualStructDefinition(size_t _numTypes)
{
	types = new VirtualType[_numTypes];
	this->_numTypes = _numTypes;
	_size = 0;
}

VirtualStructDefinition::~VirtualStructDefinition()
{
	delete[] types;
}

void VirtualStructDefinition::initalize()
{
#ifndef NDEBUG
	bool _initalized = true;
#endif
}

size_t VirtualStructDefinition::size()
{
	assert(_initalized);
	return _size;
}
