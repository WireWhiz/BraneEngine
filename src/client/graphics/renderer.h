#pragma once
#include <ecs/core/component.h>
#include <ecs/core/structMembers.h>
#include <ecs/nativeTypes/transform.h>
#include "mesh.h"
#include "material.h"

namespace graphics
{

	typedef uint32_t RendererID;
	class Renderer
	{
		Material* _material;

		std::vector<VkDescriptorSet> _descriptorSets;
		std::vector<std::vector<VkCommandBuffer>> _drawBuffers;

		NativeForEach _forEach;
	public:
		Renderer(EntityManager& em, Material* material);
		//~Renderer();
		void createDescriptorSets(size_t count);
		void createRenderBuffers(EntityManager& em, SwapChain* swapChain, std::unordered_map<MeshID,std::unique_ptr<Mesh>>& meshes, glm::mat4x4 cameraMatrix, VkCommandBufferInheritanceInfo& inheritanceInfo, size_t frame);
		std::vector<VkCommandBuffer>* getBuffers(size_t frame);
	};

	class RendererData : public NativeComponent<RendererData>
	{
		REGISTER_MEMBERS_1(disabled);
	public:
		bool disabled;

	};
}