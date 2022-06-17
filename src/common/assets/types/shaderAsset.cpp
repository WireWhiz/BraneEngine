#include "shaderAsset.h"

ShaderAsset::ShaderAsset(AssetID id, ShaderType type, std::vector<uint32_t> spirv)
{
	id = id;
	shaderType = type;
	spirv = std::move(spirv);
}

void ShaderAsset::serialize(OSerializedData& message)
{
	Asset::serialize(message);
	message << shaderType;
	message << spirv;
}

void ShaderAsset::deserialize(ISerializedData& message)
{
	Asset::deserialize(message);
	message >> shaderType;
	message >> spirv;
}

ShaderAsset::ShaderAsset()
{
	type.set(AssetType::Type::shader);
}
