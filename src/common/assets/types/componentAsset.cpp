#include "componentAsset.h"
#include "ecs/core/component.h"

#ifdef _64BIT
#define WORD_SIZE 8
#endif
#ifdef _32BIT
#define WORD_SIZE 4
#endif

ComponentAsset::ComponentAsset()
{
	type.set(AssetType::Type::component);
}

ComponentAsset::ComponentAsset(std::vector<std::unique_ptr<VirtualType>>& types, AssetID id)
{
	_size = 0;
	this->id = id;
	type.set(AssetType::Type::component);
	if (types.size() != 0)
	{

		_types.resize(types.size());
		for (size_t i = 0; i < _types.size(); i++)
		{
			_types[i] = std::move(types[i]);
		}
		for (size_t i = 0; i < _types.size(); i++)
		{
			size_t woffset = _size % WORD_SIZE;
			if (woffset != 0 && WORD_SIZE - woffset < _types[i]->size())
			{
				_size += WORD_SIZE - woffset;
			}
			_types[i]->setOffset(_size);
			_size += _types[i]->size();

		}
	}


}

ComponentAsset::ComponentAsset(std::vector<VirtualType*>& types, AssetID id, size_t size)
{
	type.set(AssetType::Type::component);
	_size = size;
	this->id = id;
	if (types.size() != 0)
	{
		_types.resize(types.size());
		for (size_t i = 0; i < _types.size(); i++)
		{
			_types[i] = std::unique_ptr<VirtualType>(types[i]);
		}
	}
}

ComponentAsset::~ComponentAsset()
{
}

size_t ComponentAsset::size() const
{
	return _size;
}

size_t ComponentAsset::getByteIndex(size_t index) const
{
	assert(index >= 0 && index < _types.size());
	return _types[index]->offset();
}


const std::vector<std::unique_ptr<VirtualType>>& ComponentAsset::types() const
{
	return _types;
}

void ComponentAsset::construct(byte* component) const
{
	for (size_t i = 0; i < _types.size(); i++)
	{
		_types[i]->construct(component + _types[i]->offset());
	}
}

void ComponentAsset::deconstruct(byte* component) const
{
	for (size_t i = 0; i < _types.size(); i++)
	{
		_types[i]->deconstruct(component + _types[i]->offset());
	}
}


void ComponentAsset::copy(byte* dest, byte* source) const
{
	for (size_t i = 0; i < _types.size(); i++)
	{
		_types[i]->copy(dest + _types[i]->offset(), source +_types[i]->offset());
	}
}

void ComponentAsset::move(byte* dest, byte* source) const
{
	for (size_t i = 0; i < _types.size(); i++)
	{
		_types[i]->move(dest + _types[i]->offset(), source + _types[i]->offset());
	}
}

void ComponentAsset::serializeComponent(OSerializedData& sdata, byte* component) const
{
	for (int i = 0; i < _types.size(); ++i)
	{
		_types[i]->serialize(sdata, component + _types[i]->offset());
	}
}

void ComponentAsset::deserializeComponent(ISerializedData& sdata, byte* component) const
{
	for (int i = 0; i < _types.size(); ++i)
	{
		 _types[i]->deserialize(sdata, component + _types[i]->offset());
	}
}

void ComponentAsset::serialize(OSerializedData& sdata)
{
	Asset::serialize(sdata);
	sdata << (uint32_t)_types.size();
	for(auto& type : _types)
	{
		sdata << (uint16_t)type->getType();
	}
}

void ComponentAsset::deserialize(ISerializedData& sdata, AssetManager& am)
{
	Asset::deserialize(sdata, am);
	uint32_t size;
	sdata.readSafeArraySize(size);
	std::vector<std::unique_ptr<VirtualType>> types;
	types.reserve(size); // Creating a local vector instead of using the classes one, so it doesn't get overwritten when we call the constructor. ID gets copied, so we don't need to worry about that.
	for(uint32_t i = 0; i < size; i++)
	{
		uint16_t type;
		sdata >> type;
		types.push_back(std::unique_ptr<VirtualType>(VirtualType::constructTypeOf((VirtualType::Type)type)));
	}
	new(this) ComponentAsset(types, id);
}


Json::Value ComponentAsset::toJson(byte* component) const
{
	VirtualComponentPtr componentPtr = VirtualComponentPtr(this, component);
	Json::Value comp;
	comp["id"] = id.string();
	comp["name"] = name;
	comp["values"];
	uint32_t index = 0;
	for(auto& variable : types())
	{
		Json::Value value;

		value["type"] = VirtualType::typeToString(variable->getType());
		switch(variable->getType())
		{
			case VirtualType::virtualUnknown:
				value["value"] = "unknown";
				break;
			case VirtualType::virtualBool:
				value["value"] = componentPtr.readVar<bool>(index++);
				break;
			case VirtualType::virtualInt:
				value["value"] = componentPtr.readVar<int32_t>(index++);
				break;
			case VirtualType::virtualInt64:
				value["value"] = componentPtr.readVar<int64_t>(index++);
				break;
			case VirtualType::virtualUInt:
				value["value"] = componentPtr.readVar<uint32_t>(index++);
				break;
			case VirtualType::virtualUInt64:
				value["value"] = componentPtr.readVar<uint64_t>(index++);
				break;
			case VirtualType::virtualFloat:
				value["value"] = componentPtr.readVar<float>(index++);
				break;
			case VirtualType::virtualString:
				value["value"] = componentPtr.readVar<std::string>(index++);
				break;
			case VirtualType::virtualAssetID:
				value["value"] = componentPtr.readVar<AssetID>(index++).string();
				break;
			case VirtualType::virtualVec3:
			{
				auto v = componentPtr.readVar<glm::vec3>(index++);
				for (uint8_t i = 0; i < 3; ++i)
				{
					auto t = v[i];
					value["value"].append(t);
				}
			}
				break;
			case VirtualType::virtualVec4:
			{
				auto v = componentPtr.readVar<glm::vec4>(index++);
				for (uint8_t i = 0; i < 4; ++i)
				{
					auto t = v[i];
					value["value"].append(t);
				}
			}
				break;
			case VirtualType::virtualMat4:
			{
				glm::mat4 m = componentPtr.readVar<glm::mat4>(index++);
				for (uint8_t i = 0; i < 16; ++i)
				{
					value["value"].append(m[i / 4][i % 4]);
				}
			}
				break;

			case VirtualType::virtualFloatVector:
			{
				auto v = componentPtr.readVar<std::vector<float>>(index++);
				for(auto e : v)
					value["value"].append(e);
			}
				break;
			case VirtualType::virtualIntVector:
			{
				auto v = componentPtr.readVar<std::vector<int>>(index++);
				for(auto e : v)
					value["value"].append(e);
			}
				break;
			case VirtualType::virtualUIntVector:
			{
				auto v = componentPtr.readVar<std::vector<uint32_t>>(index++);
				for(auto e : v)
					value["value"].append(e);
			}
				break;
		}
		comp["values"].append(value);
	}
	return comp;
}

void ComponentAsset::fromJson(Json::Value& comp, byte* component) const
{
	VirtualComponentPtr componentPtr = VirtualComponentPtr(this, component);
	assert(comp["id"] == id.string());

	uint32_t index = 0;
	for(auto& variable : types())
	{
		Json::Value value = comp["values"][index];
		switch(variable->getType())
		{
			case VirtualType::virtualUnknown:
				std::cerr << "Was unable to construct unknown type in struct" << std::endl;
				break;
			case VirtualType::virtualBool:
				componentPtr.setVar(index, value["value"].asBool());
				break;
			case VirtualType::virtualInt:
				componentPtr.setVar(index, value["value"].asInt());
				break;
			case VirtualType::virtualInt64:
				componentPtr.setVar(index, value["value"].asInt64());
				break;
			case VirtualType::virtualUInt:
				componentPtr.setVar(index, value["value"].asUInt());
				break;
			case VirtualType::virtualUInt64:
				componentPtr.setVar(index, value["value"].asUInt64());
				break;
			case VirtualType::virtualFloat:
				componentPtr.setVar(index, value["value"].asFloat());
				break;
			case VirtualType::virtualString:
				componentPtr.setVar<std::string>(index, value["value"].asString());
				break;
			case VirtualType::virtualAssetID:
				componentPtr.setVar<AssetID>(index, AssetID(value["value"].asString()));
				break;
			case VirtualType::virtualVec3:
			{
				glm::vec3 v;
				for (uint8_t i = 0; i < 3; ++i)
				{
					v[i] = value["value"][i].asFloat();
				}
				componentPtr.setVar(index, v);
			}
				break;
			case VirtualType::virtualVec4:
			{
				glm::vec4 v;
				for (uint8_t i = 0; i < 4; ++i)
				{
					v[i] = value["value"][i].asFloat();
				}
				componentPtr.setVar(index, v);
			}
				break;
			case VirtualType::virtualMat4:
			{
				glm::mat4 m;
				for (uint8_t i = 0; i < 16; ++i)
				{
					m[i / 4][i % 4] = value["value"][i].asFloat();
				}
				componentPtr.setVar(index, m);
			}
				break;

			case VirtualType::virtualFloatVector:
			{
				std::vector<float> v;
				v.reserve(value["value"].size());
				for(auto& e : value["value"])
					v.push_back(e.asFloat());
				componentPtr.setVar(index, v);
			}
				break;
			case VirtualType::virtualIntVector:
			{
				std::vector<int32_t> v;
				v.reserve(value["value"].size());
				for(auto& e : value["value"])
					v.push_back(e.asInt());
				componentPtr.setVar(index, v);
			}
				break;
			case VirtualType::virtualUIntVector:
			{
				std::vector<uint32_t> v;
				v.reserve(value["value"].size());
				for(auto& e : value["value"])
					v.push_back(e.asUInt());
				componentPtr.setVar(index, v);
			}
				break;
		}
		index++;
	}
}




