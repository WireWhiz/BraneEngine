#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../asset.h"

struct MeshPrimitive
{
	uint8_t materialIndex;
	std::vector<uint16_t> indices;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<std::vector<glm::vec2>> uvs;
	void serialize(OSerializedData& message);
	void deserialize(ISerializedData& message);
	std::vector<byte> packedData() const;
	size_t meshSize() const;
};

class MeshAsset : public IncrementalAsset
{
public:
	std::vector<MeshPrimitive> primitives;

	MeshAsset();

	void serialize(OSerializedData& message) override;
	void deserialize(ISerializedData& message, AssetManager& am) override;
	size_t meshSize() const;
};