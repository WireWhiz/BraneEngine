//
// Created by eli on 2/1/2022.
//

#include "AssetBuilder.h"
#include <ecs/nativeTypes/transform.h>
#include <ecs/nativeTypes/meshRenderer.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>



std::vector<std::unique_ptr<Asset>> AssetBuilder::buildAssembly(const std::string& name, gltfLoader& loader)
{
	std::vector<std::unique_ptr<Asset>> assets;

	std::unique_ptr<Assembly> assembly = std::make_unique<Assembly>();
	assembly->name = name;
	assembly->loadState = Asset::complete;

	std::vector<MeshAsset*> meshes = loader.extractAllMeshes();
	for(auto mesh : meshes)
	{
		std::cout << "Extracted mesh: " << mesh->name << "\n";
		assets.push_back(std::unique_ptr<MeshAsset>(mesh));
	}



	std::vector<WorldEntity> entities;
	for (Json::Value& node : loader.nodes())
	{
		WorldEntity entity;
		glm::mat4 transform = glm::mat4(1);
		if (node.isMember("matrix"))
		{

			for (uint8_t i = 0; i < node["matrix"].size(); ++i)
			{
				transform[i / 4][i % 4] = node["matrix"][i].asFloat();
			}
		} else
		{
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
				rotation = glm::mat4_cast(value) * transform;
			}
			if (node.isMember("scale"))
			{
				glm::vec3 value;
				for (uint8_t i = 0; i < 3; ++i)
				{
					value[i] = node["scale"][i].asFloat();
				}
				scale = glm::scale(glm::mat4(1), value);
			}
			if (node.isMember("translation"))
			{
				glm::vec3 value;
				for (uint8_t i = 0; i < node["translation"].size(); ++i)
				{
					value[i] = node["translation"][i].asFloat();
				}
				translation = glm::translate(glm::mat4(1), value);
			}
			transform = translation * rotation * scale;
		}
		VirtualComponent tc(TransformComponent::def());
		tc.setVar(0, transform);
		entity.components.push_back(tc);

		if (node.isMember("mesh"))
		{
			uint32_t meshIndex = node["mesh"].asUInt();

			VirtualComponent mc(MeshRendererComponent::def());
			mc.setVar(0, meshIndex);
			std::vector<uint32_t> materials;
			for(auto& primitive : loader.json()["meshes"][meshIndex]["primitives"])
				materials.push_back(primitive["material"].asUInt());
			mc.setVar(1, materials);
			entity.components.push_back(std::move(mc));
		}
		entities.push_back(entity);
	}

	uint32_t pIndex = 0;
	for (Json::Value& node : loader.nodes())
	{
		if (node.isMember("children"))
		{
			for (auto& child: node["children"])
			{
				WorldEntity& childEnt = entities[child.asUInt()];
				glm::mat4  localTransform = childEnt.components[0].readVar<glm::mat4>(0);

				VirtualComponent tc(LocalTransformComponent::def());
				tc.setVar(0, localTransform);
				tc.setVar(1, (EntityID)pIndex);
				childEnt.components.push_back(tc);
			}
		}
		pIndex++;
	}
	assembly->entities = std::move(entities);
	assets.push_back(std::move(assembly));
	return assets;
}
