//
// Created by eli on 8/20/2022.
//

#ifndef BRANEENGINE_EDITORASSEMBLYASSET_H
#define BRANEENGINE_EDITORASSEMBLYASSET_H

#include "../editorAsset.h"
#include "ecs/component.h"

class EditorAssemblyAsset : public EditorAsset
{
	Asset* buildAssembly() const;
	Asset* buildMesh(const AssetID& id) const;
public:
	static Json::Value componentToJson(VirtualComponentView component);
	static VirtualComponent jsonToComponent(Json::Value component);
	EditorAssemblyAsset(const std::filesystem::path& file, BraneProject& project);
	std::vector<std::pair<AssetID, AssetType>> containedAssets() const override;
	Asset* buildAsset(const AssetID& id) const override;

	void linkToGLTF(const std::filesystem::path& file);
	void updateEntityComponent(size_t entity, size_t component) const;
};


#endif //BRANEENGINE_EDITORASSEMBLYASSET_H
