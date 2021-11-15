#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "asset.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;

	Vertex() = default;
	Vertex(glm::vec3 pos, glm::vec3 color, glm::vec2 uv);
};

class MeshAsset : public Asset
{
public:
	std::vector<uint32_t> indices;
	std::vector<Vertex> vertices;

	MeshAsset(const AssetID& id, std::vector<uint32_t> indices, std::vector<Vertex> vertices);
	MeshAsset(net::IMessage& source);

	virtual void serialize(net::OMessage& message) override;
	virtual void deserialize(net::IMessage& message) override;
};