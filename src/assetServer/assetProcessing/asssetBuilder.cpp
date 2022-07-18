//
// Created by eli on 2/1/2022.
//

#include "assetBuilder.h"
#include <systems/transforms.h>
#include <ecs/nativeTypes/meshRenderer.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

AssetBuilder::AssemblyAssets AssetBuilder::buildAssembly(const std::string& name, gltfLoader& loader)
{
	AssemblyAssets assets;

	std::unique_ptr<Assembly> assembly = std::make_unique<Assembly>();
	assembly->name = name;
	assembly->loadState = Asset::complete;

	std::vector<MeshAsset*> meshes = loader.extractAllMeshes();
	for(auto mesh : meshes)
	{
		assets.meshes.push_back(std::unique_ptr<MeshAsset>(mesh));
	}

	std::vector<Assembly::EntityAsset> entities;
	for (Json::Value& node : loader.nodes())
	{
		Assembly::EntityAsset entity;
		glm::mat4 transform = glm::mat4(1);
		if (node.isMember("matrix"))
		{

			for (uint8_t i = 0; i < node["matrix"].size(); ++i)
			{
				transform[i / 4][i % 4] = node["matrix"][i].asFloat();
			}
			VirtualComponent tc(Transform::def());
			tc.setVar(0, transform);
			entity.components.push_back(tc);
		}
		else
		{
			VirtualComponent trs(TRS::def());
			glm::mat4 translation(1);
			glm::mat4 rotation(1);
			glm::mat4 scale(1);
			if (node.isMember("rotation"))
			{
				glm::quat value;
				for (uint8_t i = 0; i < node["rotation"].size(); ++i)
				{
					value[i] = node["rotation"][i].asFloat();
				}
				trs.setVar(1, value);
				rotation = glm::mat4_cast(value);
			}
			if (node.isMember("scale"))
			{
				glm::vec3 value;
				for (uint8_t i = 0; i < 3; ++i)
				{
					value[i] = node["scale"][i].asFloat();
				}
				trs.setVar(2, value);
				scale = glm::scale(glm::mat4(1), value);
			}
			if (node.isMember("translation"))
			{
				glm::vec3 value;
				for (uint8_t i = 0; i < node["translation"].size(); ++i)
				{
					value[i] = node["translation"][i].asFloat();
				}
				trs.setVar(0, value);
				translation = glm::translate(glm::mat4(1), value);
			}
			transform = translation * rotation * scale;
			VirtualComponent tc(Transform::def());
			tc.setVar(0, transform);
			entity.components.push_back(tc);
			entity.components.push_back(trs);
		}


		if (node.isMember("mesh"))
		{
			uint32_t meshIndex = node["mesh"].asUInt();

			VirtualComponent mc(MeshRendererComponent::def());
			mc.setVar(0, meshIndex);
			inlineUIntArray& materials = *mc.getVar<inlineUIntArray>(1);
			for(auto& primitive : loader.json()["meshes"][meshIndex]["primitives"])
				materials.push_back(primitive["material"].asUInt());

			entity.components.push_back(std::move(mc));
		}
		entities.push_back(entity);
	}

	//Link up children
	uint32_t pIndex = 0;
	for (Json::Value& node : loader.nodes())
	{
		if (node.isMember("children"))
		{
			for (auto& child: node["children"])
			{
				Assembly::EntityAsset& childEnt = entities[child.asUInt()];
				glm::mat4  localTransform = childEnt.components[0].readVar<glm::mat4>(0);

				VirtualComponent tc(LocalTransform::def());
				tc.setVar(0, localTransform);
				tc.setVar(1, (EntityID)pIndex);
				childEnt.components.push_back(tc);
			}
		}
		pIndex++;
	}

	//Register component ids
	assembly->components.push_back(Transform::def()->asset->id);
	assembly->components.push_back(LocalTransform::def()->asset->id);
	assembly->components.push_back(Children::def()->asset->id);
	assembly->components.push_back(TRS::def()->asset->id);
	assembly->components.push_back(MeshRendererComponent::def()->asset->id);

	assembly->entities = std::move(entities);
	assets.assembly = std::move(assembly);
	return std::move(assets);
}
