#pragma once
#include "../asset.h"
#include "vulkan/vulkan_core.h"
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
    std::vector<uint32_t> spirv;
    ShaderType shaderType;
    uint32_t runtimeID = -1;
	ShaderAsset();

	virtual void serialize(OutputSerializer& s) const override;
	virtual void deserialize(InputSerializer& s) override;
    VkShaderStageFlagBits vulkanShaderType() const;

#ifdef CLIENT
    void onDependenciesLoaded() override;
#endif
};