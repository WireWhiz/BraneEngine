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
	std::filesystem::path relPath = std::filesystem::relative(source, _file.parent_path()).string();
	_json.data()["source"] = relPath.string();
	std::string hash = FileManager::fileHash(source);
	bool changed = _json.data().get("lastSourceHash", "") != hash;

	if(changed)
	{
		_json.data()["lastSourceHash"] = hash;
		Runtime::log("Extracting shader attributes for " + name());
		ShaderCompiler::ShaderAttributes attributes;
		std::string glsl;
		std::filesystem::path dir = std::filesystem::path{source}.remove_filename();
		_project.editor().shaderCompiler().includeDir(dir.string());
		if(FileManager::readFile(source, glsl) && _project.editor().shaderCompiler().extractAttributes(glsl, shaderType(), attributes))
		{
			Json::Value atr;
			for(auto& ub : attributes.uniformBuffers)
			{
				Json::Value uniformBuffer;
				uniformBuffer["name"] = ub.name;
				uniformBuffer["binding"] = ub.binding;
				for(auto& m : ub.members)
				{
					Json::Value member;
					member["name"] = m.name;
					member["type"] = ShaderVariableData::typeNames.toString(m.type);
					member["layout"] = ShaderVariableData::layoutNames.toString(m.layout());
					uniformBuffer["members"].append(member);
				}
				atr["uniformBuffers"][ub.name] = uniformBuffer;
			}

			for(auto& in : attributes.inputVariables)
			{
				Json::Value input;
				input["name"] = in.name;
				input["type"] = ShaderVariableData::typeNames.toString(in.type);
				input["layout"] = ShaderVariableData::layoutNames.toString(in.layout());
				atr["inputs"][std::to_string(in.location)] = input;
			}

			for(auto& out : attributes.outputVariables)
			{
				Json::Value output;
				output["name"] = out.name;
				output["type"] = ShaderVariableData::typeNames.toString(out.type);
				output["layout"] = ShaderVariableData::layoutNames.toString(out.layout());
				atr["outputs"][std::to_string(out.location)] = output;
			}
			_json.data()["attributes"] = atr;
		}
		else
			Runtime::error("Failed extract attributes");

		_project.editor().shaderCompiler().removeIncludeDir(dir.string());
	}
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
	shader->id = _json["id"].asString();
	shader->name = name();
	shader->shaderType = shaderType();

	std::string shaderCode;
	if(!FileManager::readFile(source, shaderCode))
	{
		Runtime::error("Failed to open shader source: " + source.string());
		return nullptr;
	}
	auto& compiler = _project.editor().shaderCompiler();
	compiler.includeDir(source.remove_filename().string());
	if(!compiler.compileShader(shaderCode, shader->shaderType, shader->spirv))
	{
		delete shader;
		compiler.removeIncludeDir(source.remove_filename().string());
		return nullptr;
	}
	compiler.removeIncludeDir(source.remove_filename().string());

	ShaderCompiler::ShaderAttributes attributes;
	compiler.extractAttributes(shaderCode, shader->shaderType, attributes);
	for(auto& u : attributes.uniformBuffers)
		shader->uniforms.insert({u.name, u});

	shader->inputs = std::move(attributes.inputVariables);
	shader->outputs = std::move(attributes.outputVariables);

	return shader;
}

std::vector<std::pair<AssetID, AssetType>> EditorShaderAsset::containedAssets() const
{
	std::vector<std::pair<AssetID, AssetType>> deps;
	deps.emplace_back(AssetID{_json["id"].asString()}, AssetType::shader);
	return std::move(deps);
}

ShaderType EditorShaderAsset::shaderType() const
{
	std::filesystem::path path{_json["source"].asString()};
	auto ext = path.extension();
	if(ext == ".vert")
		return ShaderType::vertex;
	else if(ext == ".frag")
		return ShaderType::fragment;
	else if(ext == ".comp")
		return ShaderType::compute;
	Runtime::error("Unknown shader file extension: " + ext.string());
	return ShaderType::compute;
}

void EditorShaderAsset::createDefaultSource(ShaderType type)
{
	std::filesystem::path path = _file;
	path.remove_filename();

	std::filesystem::path source = std::filesystem::current_path() / "defaultAssets" / "shaders";
	switch(type)
	{
		case ShaderType::vertex:
			path /= name() + ".vert";
			source /= "default.vert";
			break;
		case ShaderType::fragment:
			path /= name() + ".frag";
			source /= "default.frag";
			break;
		default:
			Runtime::warn("Tried to create unimplemented shader type");
			return;
	}
	std::error_code ec;
	std::filesystem::copy_file(source, path, ec);
	if(ec)
	{
		Runtime::error("Could not copy default shader code: " + ec.message());
		return;
	}
	updateSource(path);
}
