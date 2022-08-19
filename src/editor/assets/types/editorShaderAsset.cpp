//
// Created by eli on 8/19/2022.
//

#include "editorShaderAsset.h"
#include "assets/types/shaderAsset.h"
#include "graphics/shader.h"
#include "runtime/runtime.h"
#include "fileManager/fileManager.h"

EditorShaderAsset::EditorShaderAsset(const std::filesystem::path& file, JsonVersionTracker& tkr) : EditorAsset(file, tkr)
{

}

Json::Value EditorShaderAsset::defaultJson() const
{
	Json::Value value = EditorAsset::defaultJson();
	value["type"] = "shader";
	return value;
}

void EditorShaderAsset::rebuildAsset(Asset* asset)
{
	EditorAsset::rebuildAsset(asset);
	ShaderAsset* shader = dynamic_cast<ShaderAsset*>(asset);
	shader->spirv = _spirv;
	shader->shaderType = _type;
}

void EditorShaderAsset::serialize(OutputSerializer& s)
{
	EditorAsset::serialize(s);
	s << _spirv << _type;
}

void EditorShaderAsset::deserialize(InputSerializer& s)
{
	EditorAsset::deserialize(s);
	s >> _spirv >> _type;
}

void EditorShaderAsset::updateFromSource(const std::filesystem::path& source)
{
	std::string fileSuffix = source.extension().string();
	auto assetPath = source;
	assetPath.replace_extension(".shader");

	graphics::SpirvHelper::Init();
	VkShaderStageFlagBits stageFlags;
	if(fileSuffix == ".vert")
	{
		stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		_type = ShaderType::vertex;
	}
	else if(fileSuffix == ".frag")
	{
		stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		_type = ShaderType::fragment;
	}
	else
	{
		Runtime::error("Unknown shader file extension: " + fileSuffix);
		return;
	}
	std::vector<char> shaderCode;
	if(!FileManager::readFile(source, shaderCode))
	{
		Runtime::error("Failed to open " + source.string());
		return;
	}
	shaderCode.resize(shaderCode.size() + 1, '\0');
	if(!graphics::CompileGLSL(stageFlags, shaderCode.data(), _spirv))
	{
		Runtime::error("Shader compilation failed for " + source.string());
		graphics::SpirvHelper::Finalize();
		return;
	}
	graphics::SpirvHelper::Finalize();
	save();
}
