//
// Created by eli on 8/31/2022.
//

#ifndef BRANEENGINE_SHADERCOMPILER_H
#define BRANEENGINE_SHADERCOMPILER_H

#include <mutex>
#include "robin_hood.h"
#include "assets/types/shaderAsset.h"

class ShaderAsset;
class ShaderCompiler
{
public:
    struct ShaderAttributes
    {
        std::vector<UniformBufferData> uniforms;
        std::vector<UniformBufferData> buffers;
        std::vector<ShaderVariableData> samplers;
        std::vector<ShaderVariableData> inputVariables;
        std::vector<ShaderVariableData> outputVariables;
    };
    ShaderCompiler();
    bool compileShader(const std::string& glsl, ShaderType type, std::vector<uint32_t>& spirv, bool optimize = true);
    bool extractAttributes(const std::string& glsl, ShaderType type, ShaderAttributes& attributes);
};


#endif //BRANEENGINE_SHADERCOMPILER_H
