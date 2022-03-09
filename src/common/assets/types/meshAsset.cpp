#include "meshAsset.h"

#include <networking/serializedData.h>

void MeshPrimitive::serialize(OSerializedData& message)
{
	message << indices << positions << normals << tangents;
	message << (uint32_t)uvs.size();
	for (size_t i = 0; i < uvs.size(); ++i)
	{
		message << uvs[i];
	}
}

void MeshPrimitive::deserialize(ISerializedData& message)
{
	message >> indices >> positions >> normals >> tangents;
	uint32_t size;
	message.readSafeArraySize(size);
	uvs.resize(size);
	for (size_t i = 0; i < size; ++i)
	{
		message >> uvs[i];
	}
}

size_t MeshPrimitive::meshSize() const
{
	size_t size = indices.size() * sizeof(uint16_t);
	if(size % 4 != 0)
		size += 2;
	size += positions.size() * sizeof(glm::vec3);
	size += normals.size() * sizeof(glm::vec3);
	size += tangents.size() * sizeof(glm::vec3);
	for(auto& uv : uvs)
	{
		size += uv.size() * sizeof(glm::vec2);
	}

	return size;
}

std::vector<byte> MeshPrimitive::packedData() const
{
	std::vector<byte> pData(meshSize());
	size_t i = 0;

	if(!indices.empty())
		std::memcpy(pData.data(), indices.data(), indices.size() * sizeof(uint16_t));
	i += indices.size() * sizeof(uint16_t);
	if(i % 4 != 0)
		i += 2;

	if(!positions.empty())
		std::memcpy(pData.data() + i, positions.data(), positions.size() * sizeof(glm::vec3));
	i += positions.size() * sizeof(glm::vec3);

	if(!normals.empty())
		std::memcpy(pData.data() + i, normals.data(), normals.size() * sizeof(glm::vec3));
	i += normals.size() * sizeof(glm::vec3);

	if(!tangents.empty())
		std::memcpy(pData.data() + i, tangents.data(), tangents.size() * sizeof(glm::vec3));
	i += tangents.size() * sizeof(glm::vec3);

	for(auto& uv : uvs)
	{
		std::memcpy(pData.data() + i, uv.data(), uv.size() * sizeof(glm::vec2));
		i += uv.size() * sizeof(glm::vec2);
	}

	return pData;
}

void MeshAsset::serialize(OSerializedData& message)
{
	Asset::serialize(message);
	message << (uint16_t)primitives.size();
	for(auto& primitive : primitives)
	{
		primitive.serialize(message);
	}
}

void MeshAsset::deserialize(ISerializedData& message, AssetManager& am)
{
	Asset::deserialize(message, am);
	uint16_t primitiveCount;
	message.readSafeArraySize(primitiveCount);
	primitives.reserve(primitiveCount);
	for (uint16_t i = 0; i < primitiveCount; ++i)
	{
		MeshPrimitive p;
		p.deserialize(message);
		primitives.push_back(p);
	}

}

MeshAsset::MeshAsset()
{
	type.set(AssetType::Type::mesh);
}

size_t MeshAsset::meshSize() const
{
	size_t size = 0;
	for(auto& m : primitives)
	{
		size += m.meshSize();
	}


	return size;
}


