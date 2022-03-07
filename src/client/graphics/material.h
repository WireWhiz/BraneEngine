#pragma once
#include "shader.h"
#include "texture.h"

#include "graphicsPipeline.h"
#include <vulkan/vulkan.h>
#include <ecs/core/Entity.h>

namespace graphics
{
	typedef uint64_t MaterialID;
	class Material
	{
		Shader* _geometryShader     = nullptr;
		Shader* _vertexShader       = nullptr;
		Shader* _fragmentShader     = nullptr;
		VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
		VkPipeline _pipeline = VK_NULL_HANDLE;

		//Descriptors
		AssetID _id;
		EntityForEachID _forEachID = ULONG_MAX;
		std::vector<Texture*> _textures;
		std::vector<VkVertexInputBindingDescription> _bindings;
		std::vector<VkVertexInputAttributeDescription> _attributes;

		VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;

		void buildDescriptorSetVars(SwapChain* swapChain);

	public:
		~Material();

		void addTextureDescriptor(Texture* texture);
		void setGeometry(Shader* shader);
		void setVertex(Shader* shader);
		void setFragment(Shader* shader);
		void addBinding(uint32_t binding, uint32_t stride);
		void addAttribute(uint32_t binding, VkFormat format, uint32_t offset);
		void buildGraphicsPipeline(SwapChain* swapChain);
		void getImageDescriptors(std::vector<VkWriteDescriptorSet>& descriptors, std::vector<VkDescriptorImageInfo>& imageInfo, VkDescriptorSet& descriptorSet);
		VkPipeline pipeline();
		VkPipelineLayout pipelineLayout();
		VkDescriptorSetLayout descriptorLayout();
		VkDescriptorPool descriptorPool();
		
	};

}