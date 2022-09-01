//
// Created by eli on 8/31/2022.
//

#include "shaderCompiler.h"
#include "assets/types/shaderAsset.h"
#include "runtime/runtime.h"

ShaderCompiler::ShaderCompiler()
{
	_options.SetOptimizationLevel(shaderc_optimization_level_performance);
}

bool ShaderCompiler::compileShader(const std::string& glsl, ShaderAsset* asset)
{
	shaderc_shader_kind kind;
	switch(asset->shaderType)
	{

		case ShaderType::vertex:
			kind = shaderc_glsl_vertex_shader;
			break;
		case ShaderType::fragment:
			kind = shaderc_glsl_fragment_shader;
			break;
		case ShaderType::geometry:
			kind = shaderc_glsl_geometry_shader;
			break;
		case ShaderType::compute:
			kind = shaderc_glsl_compute_shader;
			break;
		default:
			Runtime::error("Tried to compile unknown shader type!");
			assert(false);
			return false;
	}

	_compilerLock.lock();
	auto result = _compiler.CompileGlslToSpv(glsl, kind, glsl.c_str(), _options);
	_compilerLock.unlock();

	if(result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		Runtime::error(result.GetErrorMessage());
		return false;
	}
	asset->spirv = {result.begin(), result.end()};
	return true;
}
