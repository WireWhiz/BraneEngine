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
		ComponentID _id = ULONG_MAX;
		EnityForEachID _forEachID = ULONG_MAX;
		std::vector<Texture*> _textures;

		VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;

		void buildDescriptorSetVars(SwapChain* swapChain);

	public:
		~Material();
		
		createUniformComponent(ComponentID id, const std::vector<VirtualType> vars, size_t alignment);
		ComponentID componentID();
		void addTextureDescriptor(Texture* texture);
		void setGeometry(Shader* shader);
		void setVertex(Shader* shader);
		void setFragment(Shader* shader);
		void buildGraphicsPipeline(SwapChain* swapChain);
		void getImageDescriptors(std::vector<VkWriteDescriptorSet>& descriptors, std::vector<VkDescriptorImageInfo>& imageInfo, VkDescriptorSet& descriptorSet);
		VkPipeline pipeline();
		VkPipelineLayout pipelineLayout();
		VkDescriptorSetLayout descriptorLayout();
		VkDescriptorPool descriptorPool();
		
	};

}