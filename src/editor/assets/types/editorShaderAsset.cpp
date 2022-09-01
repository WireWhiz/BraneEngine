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
#include <mutex>
#include "editor/editor.h"

EditorShaderAsset::EditorShaderAsset(const std::filesystem::path& file, BraneProject& project) : EditorAsset(file, project)
{
	// Generate default
	if(!std::filesystem::exists(_file))
	{
		_json.data()["source"] = "";
	}
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
	if(fileSuffix == ".vert")
		shader->shaderType = ShaderType::vertex;
	else if(fileSuffix == ".frag")
		shader->shaderType = ShaderType::fragment;
	else
	{
		Runtime::error("Unknown shader file extension: " + fileSuffix);
		return nullptr;
	}
	std::string shaderCode;
	if(!FileManager::readFile(source, shaderCode))
	{
		Runtime::error("Failed to open shader source: " + source.string());
		return nullptr;
	}
	if(!_project.editor().shaderCompiler().compileShader(shaderCode, shader))
	{
		delete shader;
		return nullptr;
	}

	return shader;
}

std::vector<std::pair<AssetID, AssetType>> EditorShaderAsset::containedAssets() const
{
	return {{_json["id"].asString(), AssetType::shader}};
}
