#include "virtualType.h"
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

VirtualType* VirtualType::constructTypeOf(VirtualType::Type type)
{
	switch(type)
	{
		case virtualBool:
			return new VirtualVariable<bool>();
			break;
		case virtualInt:
			return new VirtualVariable<int>();
			break;
		case virtualUInt:
			return new VirtualVariable<uint32_t>();
			break;
		case virtualFloat:
			return new VirtualVariable<float>();
			break;
		case virtualString:
			return new VirtualVariable<std::string>();
			break;
		default:
			return nullptr;
	}
}
