//
// Created by eli on 8/31/2022.
//

#ifndef BRANEENGINE_SHADERCOMPILER_H
#define BRANEENGINE_SHADERCOMPILER_H

#include <shaderc/shaderc.hpp>
#include <mutex>

class ShaderAsset;
class ShaderCompiler
{
	std::mutex _compilerLock;
	shaderc::Compiler _compiler;
	shaderc::CompileOptions _options;
public:
	ShaderCompiler();
	bool compileShader(const std::string& glsl, ShaderAsset* asset);
};


#endif //BRANEENGINE_SHADERCOMPILER_H
