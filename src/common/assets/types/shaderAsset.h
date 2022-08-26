#pragma once
#include "../asset.h"
#include <vector>
#include <vulkan/vulkan.hpp>

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