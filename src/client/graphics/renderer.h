#pragma once
#include <ecs/core/component.h>
#include <ecs/core/structMembers.h>
#include <ecs/nativeTypes/transform.h>
#include <ecs/nativeTypes/meshRenderer.h>
#include "mesh.h"
#include "material.h"

namespace graphics
{

	typedef uint32_t RendererID;

	struct RenderObject
	{
		Mesh* mesh;
		size_t primitive;
		glm::mat4x4 transform;
	};

	class Renderer
	{
		Material* _material;

		std::vector<VkDescriptorSet> _descriptorSets;
		std::vector<std::vector<VkCommandBuffer>> _drawBuffers;

		NativeForEach _forEach;
	public:
		Renderer(Material* material);
		//~Renderer();
		void createDescriptorSets(size_t count);
		std::vector<VkCommandBuffer> createRenderBuffers(SwapChain* swapChain, std::vector<RenderObject>& meshes, glm::mat4x4 cameraMatrix, VkCommandBufferInheritanceInfo& inheritanceInfo, size_t frame);
	};
}