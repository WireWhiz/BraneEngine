#include "meshAsset.h"

#include <networking/message.h>


Vertex::Vertex(glm::vec3 pos, glm::vec3 color, glm::vec2 uv)
{
	this->pos = pos;
	this->color = color;
	this->uv = uv;
}

MeshAsset::MeshAsset(const AssetID& id, std::vector<uint32_t> indices, std::vector<Vertex> vertices)
{
	_id = id;
	this->indices = indices;
	this->vertices = vertices;
}

MeshAsset::MeshAsset(net::IMessage& source)
{
	deserialize(source);
}

void MeshAsset::serialize(net::OMessage& message)
{
	Asset::serialize(message);
	message << static_cast<uint64_t>(indices.size());
	for (size_t i = 0; i < indices.size(); i++)
	{
		message << indices[i];
	}

	message << static_cast<uint64_t>(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		message << vertices[i];
	}
	
}

void MeshAsset::deserialize(net::IMessage& message)
{
	Asset::deserialize(message);
	uint64_t numIndicies;
	message >> numIndicies;
	indices.resize(numIndicies);
	for (size_t i = 0; i < numIndicies; i++)
	{
		message >> indices[i];
	}

	uint64_t numVerticies;
	message >> numVerticies;
	vertices.resize(numVerticies);
	for (size_t i = 0; i < numVerticies; i++)
	{
		message >> vertices[i];
	}
}
