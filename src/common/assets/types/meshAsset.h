#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../asset.h"

class MeshAsset : public Asset
{
public:
	std::vector<uint16_t> indices;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<std::vector<glm::vec2>> uvs;

	MeshAsset(const AssetID& id);
	MeshAsset(ISerializedData& source);

	virtual void serialize(OSerializedData& message) override;
	virtual void deserialize(ISerializedData& message) override;
};