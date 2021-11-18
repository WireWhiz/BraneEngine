#pragma once
#include "asset.h"
#include "networking/message.h"

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
	ShaderAsset(net::IMessage& source);
	ShaderType type();

	virtual void serialize(net::OMessage& message) override;
	virtual void deserialize(net::IMessage& message) override;
};