#include "shaderAsset.h"

ShaderAsset::ShaderAsset(AssetID id, ShaderType type, std::vector<uint32_t> spirv)
{
	_id = id;
	_type = type;
	_spirv = std::move(spirv);
}

ShaderAsset::ShaderAsset(net::IMessage& source)
{
	deserialize(source);
}

ShaderType ShaderAsset::type()
{
	return _type;
}

void ShaderAsset::serialize(net::OMessage& message)
{
	Asset::serialize(message);
	message << _type;
	message << _spirv;
}

void ShaderAsset::deserialize(net::IMessage& message)
{
	Asset::deserialize(message);
	message >> _type;
	message >> _spirv;
}
