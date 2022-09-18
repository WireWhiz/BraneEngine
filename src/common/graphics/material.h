#pragma once
#include "texture.h"

#include "swapChain.h"
#include <vulkan/vulkan.h>

class MaterialAsset;
class ComponentDescription;

namespace graphics
{
    class Shader;
    class VulkanRuntime;
	class SceneRenderer;
	class Material
	{
        MaterialAsset* _asset;
		Shader* _geometryShader     = nullptr;
		Shader* _vertexShader       = nullptr;
		Shader* _fragmentShader     = nullptr;
		VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

		//Descriptors
		std::vector<Texture*> _textures;
		std::vector<VkVertexInputBindingDescription> _bindings;
		std::vector<VkVertexInputAttributeDescription> _attributes;

		std::vector<GraphicsBuffer> _transformBuffers;

		VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> _descriptorSets;

		const ComponentDescription* _component;

		void buildDescriptorSetVars(SwapChain* swapChain);
	public:
        Material(MaterialAsset* asset, VulkanRuntime* vkr);
		Material(const Material&) = delete;
		Material(Material&&);
		~Material();
		Material& operator=(Material&&);
		void addBinding(uint32_t binding, uint32_t stride);
		void addAttribute(uint32_t binding, VkFormat format, uint32_t offset);
		void buildPipelineLayout(SwapChain* swapChain);
		void initialize(size_t swapChainSize);
		GraphicsBuffer& transformBuffer(size_t frame);
		void reallocateTransformBuffer(size_t frame, size_t newSize);
		void bindTransformUniformBuffer(size_t frame, GraphicsBuffer& buffer);
		void bindPointLightBuffer(size_t frame, GraphicsBuffer& buffer);
		const ComponentDescription* component() const;
		VkPipeline pipeline(SceneRenderer* renderer) const;
		VkPipelineLayout pipelineLayout();
		VkDescriptorSetLayout descriptorLayout();
		VkDescriptorPool descriptorPool();
		VkDescriptorSet const* descriptorSet(size_t frame) const;
        MaterialAsset* asset() const;
		Shader* vertexShader() const;
		Shader* fragmentShader() const;
		
	};

}