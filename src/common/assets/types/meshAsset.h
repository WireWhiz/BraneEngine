#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../asset.h"

class MeshAsset : public IncrementalAsset
{
public:
	std::vector<uint16_t> indices;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<std::vector<glm::vec2>> uvs;

	MeshAsset();

	void serialize(OSerializedData& message) override;
	void deserialize(ISerializedData& message, AssetManager& am) override;
	size_t meshSize() const;
	std::vector<byte> packedData() const;
};