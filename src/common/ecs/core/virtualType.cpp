#include "VirtualType.h"
#include "component.h"

VirtualType::VirtualType(size_t offset)
{
	_offset = offset;
}

void VirtualType::setOffset(size_t offset)
{
	_offset = offset;
}

size_t VirtualType::offset()
{
	return _offset;
}

void VirtualType::construct(VirtualComponentPtr& vcp)
{
	construct(vcp.data());
}

void VirtualType::deconstruct(VirtualComponentPtr& vcp)
{
	deconstruct(vcp.data());
}
