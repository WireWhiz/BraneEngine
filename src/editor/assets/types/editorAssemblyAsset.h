//
// Created by eli on 8/20/2022.
//

#ifndef BRANEENGINE_EDITORASSEMBLYASSET_H
#define BRANEENGINE_EDITORASSEMBLYASSET_H

#include "../editorAsset.h"
#include "ecs/component.h"

class EditorAssemblyAsset : public EditorAsset
{
	Json::Value defaultJson() override;
	void cacheFromGLTF();
	Json::Value componentToJson(VirtualComponentView component);
	VirtualComponent jsonToComponent(Json::Value component);
public:
	EditorAssemblyAsset(const std::filesystem::path& file, BraneProject& project);
	void cacheAsset() override;
	void linkToGLTF(const std::filesystem::path& file);
};


#endif //BRANEENGINE_EDITORASSEMBLYASSET_H
