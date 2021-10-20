#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../assetID.h"

struct MeshAsset
{
	AssetID id;

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;

		Vertex(glm::vec3 pos, glm::vec3 color, glm::vec2 uv);
	};
	std::vector<uint32_t> indices;
	std::vector<Vertex> vertices;

	MeshAsset(const AssetID& id, std::vector<uint32_t> indices, std::vector<Vertex> vertices);
};