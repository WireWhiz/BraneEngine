//
// Created by eli on 8/25/2022.
//

#ifndef BRANEENGINE_EDITORMATERIALASSET_H
#define BRANEENGINE_EDITORMATERIALASSET_H

#include "../editorAsset.h"

class EditorMaterialAsset : public EditorAsset
{
public:
    EditorMaterialAsset(const std::filesystem::path& file, BraneProject& project);
    std::vector<std::pair<AssetID, AssetType>> containedAssets() const override;
    Asset* buildAsset(const AssetID& id) const override;
};


#endif //BRANEENGINE_EDITORMATERIALASSET_H
