//
// Created by eli on 8/19/2022.
//

#ifndef BRANEENGINE_EDITORSHADERASSET_H
#define BRANEENGINE_EDITORSHADERASSET_H

#include "../editorAsset.h"
#include "assets/types/shaderAsset.h"

class EditorShaderAsset : public EditorAsset
{

	Json::Value defaultJson() override;
public:
	EditorShaderAsset(const std::filesystem::path& file, BraneProject& project);
	void cacheAsset() override;
	void updateSource(const std::filesystem::path& source);
};


#endif //BRANEENGINE_EDITORSHADERASSET_H
