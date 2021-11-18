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
	message << indices;
	message << vertices;
	
}

void MeshAsset::deserialize(net::IMessage& message)
{
	Asset::deserialize(message);
	message >> indices;
	message >> vertices;
}
