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
	const size_t _trisPerIncrement = 1;
	struct IteratorData
	{
		size_t primitive;
		size_t pos;
		std::vector<bool> vertexSent;
	};
public:
	std::vector<MeshPrimitive> primitives;
	size_t pipelineID;
	bool meshUpdated;

	MeshAsset();

	void serialize(OSerializedData& message) override;
	void deserialize(ISerializedData& message, AssetManager& am) override;
	void serializeHeader(OSerializedData& sData) override;
	void deserializeHeader(ISerializedData& sData, AssetManager& am) override;
	bool serializeIncrement(OSerializedData& sData, void*& iteratorData) override;
	void deserializeIncrement(ISerializedData& sData) override;
	size_t meshSize() const;
};