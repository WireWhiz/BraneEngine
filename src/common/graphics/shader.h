#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <fstream>
#include <memory>
#include "assets/types/shaderAsset.h"

namespace graphics
{
    using ShaderID = uint64_t;
    class Shader
    {
        ShaderAsset* _asset;
        VkShaderModule _shader;
    public:
        Shader(ShaderAsset* asset);
        Shader(const Shader&) = delete;
        Shader(Shader&&);
        Shader& operator=(Shader&&);
        ~Shader();
        ShaderAsset* asset() const;
        VkShaderModule get();
        VkShaderStageFlagBits type();
        VkPipelineShaderStageCreateInfo stageInfo();

        std::vector<ShaderVariableData>& inputs() const;
    };

}