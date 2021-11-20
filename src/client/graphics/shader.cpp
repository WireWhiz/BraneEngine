#include "shader.h"
#include "graphicsDevice.h"

namespace graphics
{
	bool CompileGLSL(VkShaderStageFlagBits stage, const char* shaderCode, std::vector<unsigned int>& shaderCodeSpirV)
	{
		bool success = SpirvHelper::GLSLtoSPV(stage, shaderCode, shaderCodeSpirV);
		return success;
	}

	void SpirvHelper::Init()
	{
		glslang::InitializeProcess();
	}

	void SpirvHelper::Finalize()
	{
		glslang::FinalizeProcess();
	}

	void SpirvHelper::InitResources(TBuiltInResource& Resources)
	{
		Resources.maxLights = 32;
		Resources.maxClipPlanes = 6;
		Resources.maxTextureUnits = 32;
		Resources.maxTextureCoords = 32;
		Resources.maxVertexAttribs = 64;
		Resources.maxVertexUniformComponents = 4096;
		Resources.maxVaryingFloats = 64;
		Resources.maxVertexTextureImageUnits = 32;
		Resources.maxCombinedTextureImageUnits = 80;
		Resources.maxTextureImageUnits = 32;
		Resources.maxFragmentUniformComponents = 4096;
		Resources.maxDrawBuffers = 32;
		Resources.maxVertexUniformVectors = 128;
		Resources.maxVaryingVectors = 8;
		Resources.maxFragmentUniformVectors = 16;
		Resources.maxVertexOutputVectors = 16;
		Resources.maxFragmentInputVectors = 15;
		Resources.minProgramTexelOffset = -8;
		Resources.maxProgramTexelOffset = 7;
		Resources.maxClipDistances = 8;
		Resources.maxComputeWorkGroupCountX = 65535;
		Resources.maxComputeWorkGroupCountY = 65535;
		Resources.maxComputeWorkGroupCountZ = 65535;
		Resources.maxComputeWorkGroupSizeX = 1024;
		Resources.maxComputeWorkGroupSizeY = 1024;
		Resources.maxComputeWorkGroupSizeZ = 64;
		Resources.maxComputeUniformComponents = 1024;
		Resources.maxComputeTextureImageUnits = 16;
		Resources.maxComputeImageUniforms = 8;
		Resources.maxComputeAtomicCounters = 8;
		Resources.maxComputeAtomicCounterBuffers = 1;
		Resources.maxVaryingComponents = 60;
		Resources.maxVertexOutputComponents = 64;
		Resources.maxGeometryInputComponents = 64;
		Resources.maxGeometryOutputComponents = 128;
		Resources.maxFragmentInputComponents = 128;
		Resources.maxImageUnits = 8;
		Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
		Resources.maxCombinedShaderOutputResources = 8;
		Resources.maxImageSamples = 0;
		Resources.maxVertexImageUniforms = 0;
		Resources.maxTessControlImageUniforms = 0;
		Resources.maxTessEvaluationImageUniforms = 0;
		Resources.maxGeometryImageUniforms = 0;
		Resources.maxFragmentImageUniforms = 8;
		Resources.maxCombinedImageUniforms = 8;
		Resources.maxGeometryTextureImageUnits = 16;
		Resources.maxGeometryOutputVertices = 256;
		Resources.maxGeometryTotalOutputComponents = 1024;
		Resources.maxGeometryUniformComponents = 1024;
		Resources.maxGeometryVaryingComponents = 64;
		Resources.maxTessControlInputComponents = 128;
		Resources.maxTessControlOutputComponents = 128;
		Resources.maxTessControlTextureImageUnits = 16;
		Resources.maxTessControlUniformComponents = 1024;
		Resources.maxTessControlTotalOutputComponents = 4096;
		Resources.maxTessEvaluationInputComponents = 128;
		Resources.maxTessEvaluationOutputComponents = 128;
		Resources.maxTessEvaluationTextureImageUnits = 16;
		Resources.maxTessEvaluationUniformComponents = 1024;
		Resources.maxTessPatchComponents = 120;
		Resources.maxPatchVertices = 32;
		Resources.maxTessGenLevel = 64;
		Resources.maxViewports = 16;
		Resources.maxVertexAtomicCounters = 0;
		Resources.maxTessControlAtomicCounters = 0;
		Resources.maxTessEvaluationAtomicCounters = 0;
		Resources.maxGeometryAtomicCounters = 0;
		Resources.maxFragmentAtomicCounters = 8;
		Resources.maxCombinedAtomicCounters = 8;
		Resources.maxAtomicCounterBindings = 1;
		Resources.maxVertexAtomicCounterBuffers = 0;
		Resources.maxTessControlAtomicCounterBuffers = 0;
		Resources.maxTessEvaluationAtomicCounterBuffers = 0;
		Resources.maxGeometryAtomicCounterBuffers = 0;
		Resources.maxFragmentAtomicCounterBuffers = 1;
		Resources.maxCombinedAtomicCounterBuffers = 1;
		Resources.maxAtomicCounterBufferSize = 16384;
		Resources.maxTransformFeedbackBuffers = 4;
		Resources.maxTransformFeedbackInterleavedComponents = 64;
		Resources.maxCullDistances = 8;
		Resources.maxCombinedClipAndCullDistances = 8;
		Resources.maxSamples = 4;
		Resources.maxMeshOutputVerticesNV = 256;
		Resources.maxMeshOutputPrimitivesNV = 512;
		Resources.maxMeshWorkGroupSizeX_NV = 32;
		Resources.maxMeshWorkGroupSizeY_NV = 1;
		Resources.maxMeshWorkGroupSizeZ_NV = 1;
		Resources.maxTaskWorkGroupSizeX_NV = 32;
		Resources.maxTaskWorkGroupSizeY_NV = 1;
		Resources.maxTaskWorkGroupSizeZ_NV = 1;
		Resources.maxMeshViewCountNV = 4;
		Resources.limits.nonInductiveForLoops = 1;
		Resources.limits.whileLoops = 1;
		Resources.limits.doWhileLoops = 1;
		Resources.limits.generalUniformIndexing = 1;
		Resources.limits.generalAttributeMatrixVectorIndexing = 1;
		Resources.limits.generalVaryingIndexing = 1;
		Resources.limits.generalSamplerIndexing = 1;
		Resources.limits.generalVariableIndexing = 1;
		Resources.limits.generalConstantMatrixVectorIndexing = 1;
	}

	EShLanguage SpirvHelper::FindLanguage(const VkShaderStageFlagBits shader_type)
	{
		switch (shader_type)
		{
			case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT:
				return EShLangVertex;
			case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
				return EShLangTessControl;
			case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
				return EShLangTessEvaluation;
			case VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT:
				return EShLangGeometry;
			case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT:
				return EShLangFragment;
			case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT:
				return EShLangCompute;
			default:
				return EShLangVertex;
		}
	}

	bool SpirvHelper::GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char* pshader, std::vector<unsigned int>& spirv)
	{
		EShLanguage stage = FindLanguage(shader_type);
		glslang::TShader shader(stage);
		glslang::TProgram program;
		const char* shaderStrings[1];
		TBuiltInResource Resources = {};
		InitResources(Resources);


		// Enable SPIR-V and Vulkan rules when parsing GLSL
		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

		shaderStrings[0] = pshader;
		shader.setStrings(shaderStrings, 1);

		if (!shader.parse(&Resources, 100, false, messages))
		{
			puts(shader.getInfoLog());
			puts(shader.getInfoDebugLog());
			return false;  // something didn't work
		}

		program.addShader(&shader);

		//
		// Program-level processing...
		//

		if (!program.link(messages))
		{
			puts(shader.getInfoLog());
			puts(shader.getInfoDebugLog());
			fflush(stdout);
			return false;
		}

		glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
		return true;
	}

	Shader::Shader(std::string name, VkShaderStageFlagBits type, const std::vector<uint32_t>& spirv)
	{
		_name = std::move(name);
		_type = type;

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirv.size() * sizeof(uint32_t);
		createInfo.pCode = spirv.data();

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
		return _type;
	}
	VkPipelineShaderStageCreateInfo Shader::stageInfo()
	{
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = _type;
		shaderStageInfo.module = _shader;
		shaderStageInfo.pName = "main";

		return shaderStageInfo;
	}
	ShaderManager::ShaderManager()
	{
		SpirvHelper::Init();
	}
	ShaderManager::~ShaderManager()
	{
		SpirvHelper::Finalize();
	}
	Shader* ShaderManager::loadShader(ShaderID id)
	{
		// See if we already loaded it
		if(_shaders.count(id))
			return _shaders[id].get();

		// Check for cached spirv
		for (auto& ext : _shaderExtensions)
		{
			std::ifstream file("shaders/spirv/" + std::to_string(id) + "." + (std::string)ext.first + "b", std::ios::binary);
			if (!file.is_open())
				continue;

			uint64_t fileSize;
			file.read((char*)&fileSize, sizeof(uint64_t));

			std::vector<unsigned int> spirv(fileSize);
			file.read((char*)spirv.data(), fileSize * sizeof(unsigned int));

			// emplace shader
			_shaders[id] = std::make_unique<Shader>(std::to_string(id), ext.second, spirv);
			return _shaders[id].get();
		}

		// We didn't find a cache so try to find a source and compile
		for (auto& ext : _shaderExtensions)
		{
			std::ifstream file("shaders/src/" + std::to_string(id) + "." + (std::string)ext.first, std::ios::ate | std::ios::binary);
			if (!file.is_open())
				continue;


			size_t fileSize = file.tellg();
			char* code = new char[fileSize + 1];
			code[fileSize] = '\0'; // turn this into a c str

			//read the array into the file
			file.seekg(0);

			file.read(code, fileSize);

			file.close();

			//convert file into spirv
			std::vector<unsigned int> spirv;
			if (!CompileGLSL(ext.second, code, spirv))
				throw std::runtime_error("failed to compile shader with id: " + id);

			delete[] code;
			
			//cache spirv
			std::ofstream cacheFile("shaders/spirv/" + std::to_string(id) + "." + (std::string)ext.first + "b", std::ios::out | std::ofstream::binary);
			uint64_t spirvLength = spirv.size();
			cacheFile.write((char*)&spirvLength, sizeof(uint64_t));
			cacheFile.write((char*)spirv.data(), spirv.size() * sizeof(unsigned int));
			cacheFile.close();

			// emplace shader
			_shaders[id] = std::make_unique<Shader>(std::to_string(id), ext.second, spirv);
			return _shaders[id].get();
		}

		throw std::runtime_error("Could not find shader with id: " + id);
		return nullptr;
	}
}