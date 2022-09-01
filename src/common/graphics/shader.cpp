#include "shader.h"
#include "graphicsDevice.h"
#include "assets/types/shaderAsset.h"

namespace graphics
{
    Shader::Shader(ShaderAsset* asset) : _asset(asset)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = _asset->spirv.size() * sizeof(uint32_t);
        createInfo.pCode = _asset->spirv.data();

        if (vkCreateShaderModule(device->get(), &createInfo, nullptr, &_shader) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not create shader module!");
        }
    }
    Shader::~Shader()
	{
		vkDestroyShaderModule(device->get(), _shader, nullptr);
	}
    VkShaderModule Shader::get()
	{
		return _shader;
	}
    VkShaderStageFlagBits Shader::type()
	{
		return _asset->vulkanShaderType();
	}

	VkPipelineShaderStageCreateInfo Shader::stageInfo()
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = _asset->vulkanShaderType();
		shaderStageInfo.module = _shader;
		shaderStageInfo.pName = "main";

		return shaderStageInfo;
	}

}