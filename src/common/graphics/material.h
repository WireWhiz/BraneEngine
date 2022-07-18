#pragma once
#include "shader.h"
#include "texture.h"

#include "swapChain.h"
#include <vulkan/vulkan.h>
#include <ecs/core/entity.h>
#include "mesh.h"

namespace graphics
{
	class Renderer;
	typedef uint64_t MaterialID;
	class Material
	{
		Shader* _geometryShader     = nullptr;
		Shader* _vertexShader       = nullptr;
		Shader* _fragmentShader     = nullptr;
		VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

		//Descriptors
		std::vector<Texture*> _textures;
		std::vector<VkVertexInputBindingDescription> _bindings;
		std::vector<VkVertexInputAttributeDescription> _attributes;

		VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> _descriptorSets;

		std::vector<VirtualType::Type> _inputs;

		ComponentDescription* _component;

		void buildDescriptorSetVars(SwapChain* swapChain);
		void generateComponent();
	public:
		~Material();

		void setInputs(std::vector<VirtualType::Type> types);
		void addTextureDescriptor(Texture* texture);
		void setGeometry(Shader* shader);
		void setVertex(Shader* shader);
		void setFragment(Shader* shader);
		void addBinding(uint32_t binding, uint32_t stride);
		void addAttribute(uint32_t binding, VkFormat format, uint32_t offset);
		void buildPipelineLayout(SwapChain* swapChain);
		void initialize(size_t swapChainSize);
		ComponentDescription* component() const;
		VkPipeline pipeline(Renderer* renderer) const;
		VkPipelineLayout pipelineLayout();
		VkDescriptorSetLayout descriptorLayout();
		VkDescriptorPool descriptorPool();
		VkDescriptorSet const* descriptorSet(size_t frame) const;
		
	};

}