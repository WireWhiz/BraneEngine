//
// Created by eli on 2/1/2022.
//

#include "AssetBuilder.h"
#include <ecs/nativeTypes/transform.h>
#include <ecs/nativeTypes/meshRenderer.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

std::vector<MeshAsset*> AssetBuilder::extractMeshesFromGltf(gltfLoader& loader)
{
	return loader.extractAllMeshes();
}

std::vector<WorldEntity> AssetBuilder::extractNodes(gltfLoader& loader)
{
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
			if (node.isMember("position"))
			{
				glm::vec3 value;
				for (uint8_t i = 0; i < node["position"].size(); ++i)
				{
					value[i] = node["position"][i].asFloat();
				}
				transform = glm::translate(transform, value);
			}
			if (node.isMember("rotation"))
			{
				glm::quat value;
				for (uint8_t i = 0; i < node["rotation"].size(); ++i)
				{
					value[i] = node["rotation"][i].asFloat();
				}
				transform *= glm::toMat4(value);
			}

		}
		VirtualComponent tc(comps::TransformComponent::def());
		tc.setVar(0, transform);
		entity.components.push_back(tc);

		if (node.isMember("mesh"))
		{
			VirtualComponent mc(comps::MeshRendererComponent::def());
			mc.setVar(0, node["mesh"].asUInt());
			entity.components.push_back(mc);
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
				glm::mat4  transform = childEnt.components[0].readVar<glm::mat4>(0);

				VirtualComponent tc(comps::LocalTransformComponent::def());
				tc.setVar(0, transform);
				tc.setVar(1, (EntityID)pIndex);
				childEnt.components[0] = tc;
			}
		}
		pIndex++;
	}
	return entities;
}
