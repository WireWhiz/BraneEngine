#include "meshAsset.h"

#include <networking/serializedData.h>

MeshAsset::MeshAsset(const AssetID& id)
{
	_header.id = id;
}

MeshAsset::MeshAsset(ISerializedData& source)
{
	deserialize(source);
}

void MeshAsset::serialize(OSerializedData& message)
{
	Asset::serialize(message);
	message << indices << positions << normals << tangents;
	message << (uint32_t)uvs.size();
	for (size_t i = 0; i < uvs.size(); ++i)
	{
		message << uvs[i];
	}
	
}

void MeshAsset::deserialize(ISerializedData& message)
{
	Asset::deserialize(message);
	message >> indices >> positions >> normals >> tangents;
	uint32_t size;
	message.readSafeArraySize(size);
	uvs.resize(size);
	for (size_t i = 0; i < size; ++i)
	{
		message >> uvs[i];
	}
}
