#pragma once

#include "assets/asset.h"
#include "ecs/virtualType.h"
class ComponentAsset;
class MaterialAsset : public Asset
{
public:
    MaterialAsset();
    std::vector<AssetID> textures;
    AssetID vertexShader;
    AssetID fragmentShader;
    AssetID inputComponent;

    void serialize(OutputSerializer& s) const override;
    void deserialize(InputSerializer& s) override;
    std::vector<AssetDependency> dependencies() const override;

#ifdef CLIENT
    uint32_t runtimeID = -1;
    void onDependenciesLoaded() override;
#endif
};