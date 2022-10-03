//
// Created by eli on 5/29/2022.
//

#ifndef BRANEENGINE_SCENERENDERER_H
#define BRANEENGINE_SCENERENDERER_H
#include "renderer.h"
#include "glm/gtx/quaternion.hpp"
#include "graphicsBuffer.h"

class EntityManager;
namespace graphics{
    class VulkanRuntime;
    class SceneRenderer : public Renderer
    {
        VulkanRuntime& _vkr;
        EntityManager& _em;
        std::unordered_map<const Material*, VkPipeline> _cachedPipelines;

        void rebuild() override;
        VkPipeline getPipeline(const Material* mat);
        std::vector<GraphicsBuffer> _renderDataBuffers;
        std::vector<GraphicsBuffer> _pointLights;

        struct alignas(16) PointLightData
        {
            alignas(16) glm::vec3 position;
            alignas(16) glm::vec4 color;
        };

        void updateLights(size_t frame, std::vector<PointLightData>& lights);

    public:
        glm::vec3 position;
        glm::quat rotation;
        float fov = 45;
        SceneRenderer(SwapChain& swapChain, VulkanRuntime* vkr, EntityManager* em);
        ~SceneRenderer() override;
        void render(VkCommandBuffer cmdBuffer) override;
        glm::mat4 transformMatrix() const;
        glm::mat4 perspectiveMatrix() const;

        void reloadMaterial(Material* material);
    };
}



#endif //BRANEENGINE_SCENERENDERER_H
