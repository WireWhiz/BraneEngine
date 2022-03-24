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
	pipelineID = -1;
	type.set(AssetType::Type::mesh);
	_incrementCount = 1;
	meshUpdated = false;
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

void MeshAsset::serializeHeader(OSerializedData& sData)
{
	IncrementalAsset::serializeHeader(sData);
	sData << (uint32_t)primitives.size();
	for(auto& primitive : primitives)
	{
		sData << (uint32_t)primitive.indices.size();
		sData << (uint32_t)primitive.positions.size(); // Number of vertices
		sData << !primitive.normals.empty();
		sData << !primitive.tangents.empty();
		sData << (uint8_t)primitive.uvs.size(); // We only need the number of
	}
}

void MeshAsset::deserializeHeader(ISerializedData& sData, AssetManager& am)
{
	IncrementalAsset::deserializeHeader(sData, am);
	uint32_t primitiveCount;
	sData >> primitiveCount;
	primitives.resize(primitiveCount);//Preallocate all the data that we are going to need for the mesh
	for(auto& primitive : primitives)
	{
		uint32_t indices;
		uint32_t vertices;
		bool normals;
		bool tangents;
		uint8_t uvs;

		sData >> indices >> vertices >> normals >> tangents >> uvs;

		primitive.indices.resize(indices);

		glm::vec3 nan3 = {0, 0, 0};
		primitive.positions.resize(vertices, nan3);
		if(normals)
			primitive.normals.resize(vertices, nan3);
		if(tangents)
			primitive.tangents.resize(vertices, nan3);
		primitive.uvs.resize(uvs);
		for(auto& uv : primitive.uvs)
		{
			uv.resize(vertices, nan3);
		}
	}
	loadState = partial;
}
//For now, we're just testing the header first, data later setup, so all meshes will be sent as only one increment.
bool MeshAsset::serializeIncrement(OSerializedData& sData, void*& iteratorData)
{
	IncrementalAsset::serializeIncrement(sData, iteratorData);
	if(!iteratorData)
		iteratorData = new IteratorData{};
	auto* itr = (IteratorData*)iteratorData;
	auto& primitive = primitives[itr->primitive];
	sData << (uint16_t)itr->primitive;
	if(!itr->indicesSent)
	{
		sData << true << primitive.materialIndex << primitive.indices;
		itr->indicesSent = true;
	}
	else
	{
		size_t start = itr->pos;
		size_t count = std::min(primitive.positions.size(), _verticiesPerIncrement + start) - start;
		sData << false << (uint32_t)start << (uint32_t)count;
		for(size_t i = start; i < start + count; i++)
		{
			sData << primitive.positions[i];
			if(!primitive.normals.empty())
				sData << primitive.normals[i];
			if(!primitive.tangents.empty())
				sData << primitive.tangents[i];
			for(auto& uv : primitive.uvs)
			{
				sData << uv[i];
			}
		}
		std::cout << "Serialized start: " << start << " count: " << count << " primitive: " << itr->primitive << std::endl;
		itr->pos += count;
		if(start + count >= primitive.positions.size())
		{
			itr->primitive++;
			itr->pos = 0;
			itr->indicesSent = false;
			if(itr->primitive >= primitives.size())
				return false;
		}
	}
	return true;
}

void MeshAsset::deserializeIncrement(ISerializedData& sData)
{
	uint16_t pIndex;
	sData >> pIndex;
	auto& primitive = primitives[pIndex];

	bool settingIndices;
	sData >> settingIndices;
	if(settingIndices)
	{
		sData >> primitive.materialIndex >> primitive.indices;
	}
	else
	{
		uint32_t start, count;
		sData >> start >> count;
		for(size_t i = start; i < start + count; i++)
		{
			sData >> primitive.positions[i];
			if(!primitive.normals.empty())
				sData >> primitive.normals[i];
			if(!primitive.tangents.empty())
				sData >> primitive.tangents[i];
			for(auto& uv : primitive.uvs)
			{
				sData >> uv[i];
			}
		}
	}
	meshUpdated = true;
}


