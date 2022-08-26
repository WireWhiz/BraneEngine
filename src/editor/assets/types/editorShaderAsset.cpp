//
// Created by eli on 8/19/2022.
//

#include "editorShaderAsset.h"
#include "assets/types/shaderAsset.h"
#include "graphics/shader.h"
#include "runtime/runtime.h"
#include "fileManager/fileManager.h"
#include "editor/braneProject.h"
#include "utility/hex.h"

EditorShaderAsset::EditorShaderAsset(const std::filesystem::path& file, BraneProject& project) : EditorAsset(file, project)
{

}

Json::Value EditorShaderAsset::defaultJson()
{
	Json::Value value = EditorAsset::defaultJson();
	value["source"] = "";
	return value;
}

void EditorShaderAsset::updateSource(const std::filesystem::path& source)
{
	_json.data()["source"] = std::filesystem::relative(source, _file.parent_path()).string();
	save();
}

Asset* EditorShaderAsset::buildAsset(const AssetID& id) const
{
	assert(id.string() == _json["id"].asString());
	if(_json["source"].asString().empty())
	{
		Runtime::error("Shader source not set for " + _json["name"].asString());
		return nullptr;
	}
	std::filesystem::path source = _file.parent_path() / _json["source"].asString();
	std::string fileSuffix = source.extension().string();

	ShaderAsset* shader = new ShaderAsset();
	shader->id.parseString(_json["id"].asString());
	shader->name = name();
	graphics::SpirvHelper::Init();
	VkShaderStageFlagBits stageFlags;
	if(fileSuffix == ".vert")
	{
		stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		shader->shaderType = ShaderType::vertex;
	}
	else if(fileSuffix == ".frag")
	{
		stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader->shaderType = ShaderType::fragment;
	}
	else
	{
		Runtime::error("Unknown shader file extension: " + fileSuffix);
		return nullptr;
	}
	std::vector<char> shaderCode;
	if(!FileManager::readFile(source, shaderCode))
	{
		Runtime::error("Failed to open shader source: " + source.string());
		return nullptr;
	}
	//Add null termination char
	shaderCode.resize(shaderCode.size() + 1, '\0');
	if(!graphics::CompileGLSL(stageFlags, shaderCode.data(), shader->spirv))
	{
		Runtime::error("Shader compilation failed for " + source.string());
		graphics::SpirvHelper::Finalize();
		return nullptr;
	}
	graphics::SpirvHelper::Finalize();

	return shader;
}

std::vector<std::pair<AssetID, AssetType>> EditorShaderAsset::containedAssets() const
{
	return {{_json["id"].asString(), AssetType::shader}};
}
