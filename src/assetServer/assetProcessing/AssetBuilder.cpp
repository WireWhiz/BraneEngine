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
	extractNodes_recursive(loader, 0, 0, entities);
	return entities;
}

void AssetBuilder::extractNodes_recursive(gltfLoader& loader, uint32_t nodeIndex, uint32_t parentIndex, std::vector<WorldEntity>& entities)
{
	Json::Value& node = loader.nodes()[nodeIndex];
	{ // Scope to reduce memory usage from recursion
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
		if (nodeIndex != parentIndex) //If we have a parent use a local transform, otherwise use global
		{
			VirtualComponent tc(comps::LocalTransformComponent::def());
			tc.setVar(0, transform);
			tc.setVar(1, (EntityID)parentIndex);
			entity.components.push_back(tc);
		} else
		{
			VirtualComponent tc(comps::TransformComponent::def());
			tc.setVar(0, transform);
			entity.components.push_back(tc);
		}

		if (node.isMember("mesh"))
		{
			VirtualComponent mc(comps::MeshRendererComponent::def());
			mc.setVar(0, node["mesh"].asUInt());
			entity.components.push_back(mc);
		}

		entities.push_back(entity);
	}
	if(node.isMember("children"))
	{
		for(auto& child : node["children"])
		{
			extractNodes_recursive(loader, child.asUInt(), nodeIndex, entities);
		}
	}




}
