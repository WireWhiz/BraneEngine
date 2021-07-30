#include "shaders.h"

bool graphics::LoadShader(VkShaderStageFlagBits stage, const char* shaderCode)
{

	std::vector<unsigned int> shaderCodeSpirV;
	bool success = SpirvHelper::GLSLtoSPV(stage, shaderCode, shaderCodeSpirV);

	return success;
}