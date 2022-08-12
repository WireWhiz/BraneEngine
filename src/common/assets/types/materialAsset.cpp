#include "materialAsset.h"
#include <utility/serializedData.h>
#include "componentAsset.h"
#include "graphics/graphics.h"

void MaterialAsset::serialize(OutputSerializer& s)
{
    Asset::serialize(s);
    s << vertexShader << fragmentShader << textures << inputComponent;
}

void MaterialAsset::deserialize(InputSerializer& s)
{
    Asset::deserialize(s);
    s >> vertexShader >> fragmentShader >> textures >> inputComponent;
}

MaterialAsset::MaterialAsset()
{
    type.set(AssetType::material);
}

std::vector<AssetDependency> MaterialAsset::dependencies() const
{
    std::vector<AssetDependency> deps;
    if(!vertexShader.serverAddress.empty())
        deps.push_back({vertexShader, false});
    if(!fragmentShader.serverAddress.empty())
        deps.push_back({fragmentShader, false});
    if(!inputComponent.serverAddress.empty())
        deps.push_back({inputComponent, false});
    return deps;
}

#ifdef CLIENT
void MaterialAsset::onDependenciesLoaded()
{
    auto* vkr = Runtime::getModule<graphics::VulkanRuntime>();
    vkr->addAsset(this);
}
#endif
