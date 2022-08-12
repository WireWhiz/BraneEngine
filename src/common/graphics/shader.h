#pragma once

#ifdef _DEBUG
#define DEBUG_DEFINED
#endif

#include <SPIRV/GlslangToSpv.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <fstream>
#include <memory>

class ShaderAsset;
namespace graphics
{
	struct SpirvHelper
	{
		static void Init();

		static void Finalize();

		static void InitResources(TBuiltInResource& Resources);

		static EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type);

		static bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char* pshader, std::vector<unsigned int>& spirv);
	};

	bool CompileGLSL(VkShaderStageFlagBits stage, const char* shaderCode, std::vector<unsigned int>& shaderCodeSpirV);

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