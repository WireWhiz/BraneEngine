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
    size_t runtimeID = -1;

    void serialize(OutputSerializer& s) override;
    void deserialize(InputSerializer& s) override;
    std::vector<AssetDependency> dependencies() const override;

#ifdef CLIENT
    void onDependenciesLoaded() override;
#endif
};