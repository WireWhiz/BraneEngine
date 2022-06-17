//
// Created by eli on 5/29/2022.
//

#ifndef BRANEENGINE_MESHRENDERER_H
#define BRANEENGINE_MESHRENDERER_H
#include "renderer.h"

namespace graphics{
	class VulkanRuntime;
	class MeshRenderer : public Renderer
	{
		VulkanRuntime& _vkr;
		EntityManager& _em;
		std::unordered_map<const Material*, VkPipeline> _cachedPipelines;

		void rebuild() override;
		VkPipeline getPipeline(const Material* mat);

	public:
		glm::vec3 position;
		glm::quat rotation;
		float fov = 45;
		MeshRenderer(SwapChain& swapChain, VulkanRuntime* vkr, EntityManager* em);
		~MeshRenderer() override;
		void render(VkCommandBuffer cmdBuffer) override;
	};
}



#endif //BRANEENGINE_MESHRENDERER_H
