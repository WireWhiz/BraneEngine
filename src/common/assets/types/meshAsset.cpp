#include "meshAsset.h"


MeshAsset::Vertex::Vertex(glm::vec3 pos, glm::vec3 color, glm::vec2 uv)
{
	this->pos = pos;
	this->color = color;
	this->uv = uv;
}

MeshAsset::MeshAsset(const AssetID& id, std::vector<uint32_t> indices, std::vector<Vertex> vertices)
{
	this->id = id;
	this->indices = indices;
	this->vertices = vertices;
}
