//
// Created by eli on 2/2/2022.
//

#include "assembly.h"

void WorldEntity::serialize(OSerializedData& message)
{
	message << components.size();
	for (int i = 0; i < components.size(); ++i)
	{
		message << components[i].def()->id();
		components[i].def()->serialize(message, components[i].data());
	}
}

void WorldEntity::deserialize(ISerializedData& message)
{
	uint32_t size;
	message.readSafeArraySize(size);
	components.reserve(size);
	for (uint32_t i = 0; i < size; ++i)
	{
		AssetID componentID;
		message >> componentID;
		assert(false && "Write code here");
		// Here we retrieve the asset id from somewhere and then use it to read the serialized data
		//VirtualComponent component(assetDefiniton);
		//component.def()->deserialize(message, component.data());
		//components.push_back(std::move(component));
	}
}

void Assembly::serialize(OSerializedData& message)
{
	Asset::serialize(message);
	message << scripts << meshes << textures;
	message << (uint32_t)data.size();
	for (uint32_t i = 0; i < data.size(); ++i)
	{
		data[i].serialize(message);
	}
}

void Assembly::deserialize(ISerializedData& message)
{
	Asset::deserialize(message);
	message >> scripts >> meshes >> textures;
	uint32_t size;
	message.readSafeArraySize(size);
	data.resize(size);
	for (uint32_t i = 0; i < data.size(); ++i)
	{
		data[i].deserialize(message);
	}
}


