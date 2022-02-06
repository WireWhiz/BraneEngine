#include "shaderAsset.h"

ShaderAsset::ShaderAsset(AssetID id, ShaderType type, std::vector<uint32_t> spirv)
{
	_header.id = id;
	_type = type;
	_spirv = std::move(spirv);
}

ShaderAsset::ShaderAsset(ISerializedData& source)
{
	deserialize(source);
}

ShaderType ShaderAsset::type()
{
	return _type;
}

void ShaderAsset::serialize(OSerializedData& message)
{
	Asset::serialize(message);
	message << _type;
	message << _spirv;
}

void ShaderAsset::deserialize(ISerializedData& message)
{
	Asset::deserialize(message);
	message >> _type;
	message >> _spirv;
}
