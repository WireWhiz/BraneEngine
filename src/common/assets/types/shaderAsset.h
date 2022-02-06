#pragma once
#include "../asset.h"
#include "networking/serializedData.h"

enum class ShaderType
{
	vertex,
	fragment,
	geometry,
	compute
};

class ShaderAsset : public Asset
{
	ShaderType _type;
	std::vector<uint32_t> _spirv;
public:
	ShaderAsset(AssetID id, ShaderType type, std::vector<uint32_t> spirv);
	ShaderAsset(ISerializedData& source);
	ShaderType type();

	virtual void serialize(OSerializedData& message) override;
	virtual void deserialize(ISerializedData& message) override;
};