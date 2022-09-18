//
// Created by eli on 8/31/2022.
//

#ifndef BRANEENGINE_SHADERCOMPILER_H
#define BRANEENGINE_SHADERCOMPILER_H

#include <shaderc/shaderc.hpp>
#include <mutex>
#include "robin_hood.h"
#include "assets/types/shaderAsset.h"
#include "libshaderc_util/file_finder.h"

class ShaderAsset;
class ShaderCompiler
{
	shaderc_util::FileFinder _fileFinder;
public:
	struct ShaderAttributes
	{
		std::vector<UniformBufferData> uniformBuffers;
		std::vector<ShaderVariableData> samplers;
		std::vector<ShaderVariableData> inputVariables;
		std::vector<ShaderVariableData> outputVariables;
	};
	ShaderCompiler();
	void includeDir(const std::string& path);
	void removeIncludeDir(const std::string& path);
	bool compileShader(const std::string& glsl, ShaderType type, std::vector<uint32_t>& spirv, bool optimize = true);
	bool extractAttributes(const std::string& glsl, ShaderType type, ShaderAttributes& attributes);
};


#endif //BRANEENGINE_SHADERCOMPILER_H
