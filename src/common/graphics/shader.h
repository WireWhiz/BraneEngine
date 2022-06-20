#pragma once

#ifdef _DEBUG
#define DEBUG_DEFINED
#endif

#include <SPIRV/GlslangToSpv.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <fstream>
#include <memory>


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

	typedef uint64_t ShaderID;

	class Shader
	{
		VkShaderModule _shader;
		VkShaderStageFlagBits _type;
		std::string _name;
	public:
		Shader(std::string name, VkShaderStageFlagBits type, const std::vector<uint32_t>& spirv);
		~Shader();
		VkShaderModule get();
		VkShaderStageFlagBits type();
		VkPipelineShaderStageCreateInfo stageInfo();
	};

	class ShaderManager
	{
		std::array<std::pair<std::string_view, VkShaderStageFlagBits>, 2> _shaderExtensions = {std::make_pair("vert", VK_SHADER_STAGE_VERTEX_BIT), std::make_pair("frag" , VK_SHADER_STAGE_FRAGMENT_BIT)};
		std::unordered_map<ShaderID, std::unique_ptr<Shader>> _shaders;
	public:
		ShaderManager();
		~ShaderManager();
		Shader* loadShader(ShaderID id);
	};
}