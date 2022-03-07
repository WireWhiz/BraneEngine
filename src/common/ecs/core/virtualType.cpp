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
		case virtualUnknown:
			return nullptr;
		case virtualBool:
			return new VirtualVariable<bool>();
		case virtualInt:
			return new VirtualVariable<int32_t>();
		case virtualInt64:
			return new VirtualVariable<int64_t>();
		case virtualUInt:
			return new VirtualVariable<uint32_t>();
		case virtualUInt64:
			return new VirtualVariable<uint64_t>();
		case virtualFloat:
			return new VirtualVariable<float>();
		case virtualString:
			return new VirtualVariable<std::string>();
		case virtualVec3:
			return new VirtualVariable<glm::vec3>();
		case virtualVec4:
			return new VirtualVariable<glm::vec4>();
		case virtualMat4:
			return new VirtualVariable<glm::mat4>();
		default:
			return nullptr;
	}
}

std::string VirtualType::typeToString(Type type)
{
	static const std::unordered_map<Type, const std::string> _toStringMap = {
			{virtualUnknown, "unknown"},
			{virtualBool,    "bool"   },
			{virtualInt,     "int"    },
			{virtualInt64,   "int64"  },
			{virtualUInt,    "uint"   },
			{virtualUInt64,  "uint64" },
			{virtualFloat,   "float"  },
			{virtualString,  "string" },
			{virtualVec3,    "vec3"   },
			{virtualMat4,    "mat4"   }
	};
	assert(_toStringMap.count(type));
	return _toStringMap.at(type);
}

VirtualType::Type VirtualType::stringToType(const std::string& type)
{
	static const std::unordered_map<std::string, Type> _toStringMap = {
			{"unknown",  virtualUnknown},
			{"bool",        virtualBool},
			{"int",          virtualInt},
			{"int64",      virtualInt64},
			{"uint",        virtualUInt},
			{"uint64",    virtualUInt64},
			{"float",      virtualFloat},
			{"string",    virtualString},
			{"vec3",        virtualVec3},
			{"mat4",        virtualMat4}
	};
	assert(_toStringMap.count(type));
	return _toStringMap.at(type);
}
