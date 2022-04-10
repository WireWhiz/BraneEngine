#include "meshAsset.h"

#include <utility/serializedData.h>

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

void MeshAsset::toFile(MarkedSerializedData& sData)
{
	Asset::toFile(sData);
	sData.writeAttribute("primitives", (uint32_t)primitives.size());

	for (uint16_t i = 0; i < primitives.size(); ++i)
	{
		std::string pName = "primitive_" + std::to_string(i);
		MeshPrimitive& p = primitives[i];
		sData.writeAttribute(pName + "_indices", p.indices);
		sData.writeAttribute(pName + "_positions", p.positions);
		sData.writeAttribute(pName + "_tangents", p.tangents);
		sData.writeAttribute(pName + "_normals", p.normals);
		sData.writeAttribute(pName + "_uvs", (uint8_t)p.uvs.size());
		for (int j = 0; j < p.uvs.size(); ++j)
		{
			sData.writeAttribute(pName + "_uv_" + std::to_string(j), p.uvs[j]);
		}
	}

}

void MeshAsset::fromFile(MarkedSerializedData& sData, AssetManager& am)
{
	Asset::fromFile(sData, am);
	uint32_t count;
	sData.readAttribute("primitives", count);
	primitives.resize(count);
	for (uint16_t i = 0; i < primitives.size(); ++i)
	{
		std::string pName = "primitive_" + std::to_string(i);
		MeshPrimitive& p = primitives[i];
		sData.readAttribute(pName + "_indices", p.indices);
		sData.readAttribute(pName + "_positions", p.positions);
		sData.readAttribute(pName + "_tangents", p.tangents);
		sData.readAttribute(pName + "_normals", p.normals);
		uint8_t uvs;
		sData.readAttribute(pName + "_uvs", uvs);
		p.uvs.resize(uvs);
		for (int j = 0; j < p.uvs.size(); ++j)
		{
			sData.readAttribute(pName + "_uv_" + std::to_string(j), p.uvs[j]);
		}
	}
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
bool MeshAsset::serializeIncrement(OSerializedData& sData, SerializationContext* iteratorData)
{
	IncrementalAsset::serializeIncrement(sData, iteratorData);

	auto* itr = (SContext*)iteratorData;
	auto& primitive = primitives[itr->primitive];
	sData << (uint16_t)itr->primitive;

	uint32_t start = itr->pos;
	uint32_t end = std::min(primitive.indices.size(), _trisPerIncrement * 3 + start);
	sData << start << end;

	for (size_t i = start; i < end; ++i)
	{
		uint16_t index = primitive.indices[i];
		sData << index;
		sData << !(bool)itr->vertexSent[index]; //We have to cast these because vector returns a custom wrapper for references that's not "trivially copyable"
		// If we haven't sent this vertex, send it.
		if(!itr->vertexSent[index])
		{
			sData << (glm::vec3&)primitive.positions[index];
			if(!primitive.normals.empty())
				sData << (glm::vec3&)primitive.normals[index];
			if(!primitive.tangents.empty())
				sData << (glm::vec3&)primitive.tangents[index];
			for(auto& uv : primitive.uvs)
			{
				sData << (glm::vec2&)uv[index];
			}
			itr->vertexSent[index] = true;
		}
	}
	itr->pos = end;

	if(itr->pos == primitive.indices.size())
	{
		itr->primitive++;
		itr->pos = 0;

		if(itr->primitive >= primitives.size())
			return false;
		itr->vertexSent.clear();
		itr->vertexSent.resize(primitives[itr->primitive].positions.size());
	}

	return true;
}

void MeshAsset::deserializeIncrement(ISerializedData& sData)
{
	uint16_t pIndex;
	sData >> pIndex;
	auto& primitive = primitives[pIndex];

	uint32_t start, end;
	sData >> start >> end;

	for (size_t i = start; i < end; ++i)
	{
		uint16_t index;
		sData >> index;
		primitive.indices[i] = index;

		bool vertexSent;
		sData >> vertexSent;
		// If we haven't sent this vertex, send it.
		if(vertexSent)
		{
			sData >> (glm::vec3&)primitive.positions[index];
			if(!primitive.normals.empty())
				sData >> (glm::vec3&)primitive.normals[index];
			if(!primitive.tangents.empty())
				sData >> (glm::vec3&)primitive.tangents[index];
			for(auto& uv : primitive.uvs)
			{
				sData >> (glm::vec2&)uv[index];
			}
		}
	}
	meshUpdated = true;
}

std::unique_ptr<IncrementalAsset::SerializationContext> MeshAsset::createContext() const
{
    std::unique_ptr<SContext> sc = std::make_unique<SContext>();
    sc->vertexSent.resize(primitives[0].positions.size());
    return std::move(sc);
}


