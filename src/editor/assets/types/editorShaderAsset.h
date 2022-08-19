//
// Created by eli on 8/19/2022.
//

#ifndef BRANEENGINE_EDITORSHADERASSET_H
#define BRANEENGINE_EDITORSHADERASSET_H

#include "../editorAsset.h"
#include "assets/types/shaderAsset.h"

class EditorShaderAsset : public EditorAsset
{
	std::vector<uint32_t> _spirv;
	ShaderType _type;
	Json::Value defaultJson() const override;
	void serialize(OutputSerializer& s) override;
	void deserialize(InputSerializer& s) override;
public:
	EditorShaderAsset(const std::filesystem::path& file, JsonVersionTracker& tkr);
	void rebuildAsset(Asset *asset) override;
	void updateFromSource(const std::filesystem::path& source);
};


#endif //BRANEENGINE_EDITORSHADERASSET_H
