#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <fstream>
#include <memory>

class ShaderAsset;
namespace graphics
{
	using ShaderID = uint64_t;
	class Shader
	{
		ShaderAsset* _asset;
        VkShaderModule _shader;
	public:
        Shader(ShaderAsset* asset);
		~Shader();
		VkShaderModule get();
		VkShaderStageFlagBits type();
		VkPipelineShaderStageCreateInfo stageInfo();
	};

}