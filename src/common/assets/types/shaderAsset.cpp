#include "shaderAsset.h"
#include "utility/serializedData.h"
#include "graphics/graphics.h"
#include "runtime/runtime.h"

void ShaderAsset::serialize(OutputSerializer& s)
{
	Asset::serialize(s);
	s << shaderType;
	s << spirv;
}

void ShaderAsset::deserialize(InputSerializer& s)
{
	Asset::deserialize(s);
	s >> shaderType;
	s >> spirv;
}

ShaderAsset::ShaderAsset()
{
	type.set(AssetType::Type::shader);
}

#ifdef CLIENT
void ShaderAsset::onDependenciesLoaded()
{
    auto* vkr = Runtime::getModule<graphics::VulkanRuntime>();
    vkr->addAsset(this);
}
#endif

VkShaderStageFlagBits ShaderAsset::vulkanShaderType() const
{
    switch(shaderType)
    {

        case ShaderType::vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderType::fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderType::geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderType::compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
    }
    assert(false && "Unreachable");
    return (VkShaderStageFlagBits)0;
}
