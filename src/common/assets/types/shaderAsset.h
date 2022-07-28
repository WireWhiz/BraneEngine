#pragma once
#include "../asset.h"
#include <vector>

enum class ShaderType
{
	vertex,
	fragment,
	geometry,
	compute
};

class ShaderAsset : public Asset
{
public:
	ShaderType shaderType;
	std::vector<uint32_t> spirv;
	ShaderAsset(AssetID id, ShaderType type, std::vector<uint32_t> spirv);
	ShaderAsset();

	virtual void serialize(OutputSerializer& message) override;
	virtual void deserialize(InputSerializer& message) override;
};