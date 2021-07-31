#include "shaders.h"

bool graphics::CompileGLSL(VkShaderStageFlagBits stage, const char* shaderCode, std::vector<unsigned int>& shaderCodeSpirV)
{
	bool success = SpirvHelper::GLSLtoSPV(stage, shaderCode, shaderCodeSpirV);
	return success;
}