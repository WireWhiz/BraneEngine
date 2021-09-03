#pragma once

#include "window.h"
#include "graphicsBuffer.h"
#include "mesh.h"
#include "shader.h"
#include "swapChain.h"
#include "texture.h"
#include "../threadPool.h"
#include "../clock.h"

#include <unordered_map>
#include <thread>
#include <iostream>
#include <fstream>
#include <vector>


namespace graphics
{
	class Pipeline
	{
		SwapChain* _swapChain;
		Texture* _texture;

		VkPipelineLayout _pipelineLayout;
		GraphicsBuffer* _graphicsBuffer;

		VkDescriptorPool _descriptorPool;
		std::vector<VkDescriptorSet> _descriptorSets;

		std::vector<GraphicsBuffer> _uniformBuffers;
		std::vector<VkCommandBuffer> _commandBuffers;


		void createDescriptorSetLayout();
		void createGraphicsPipline();
		void createTextureImage();
		void createModelBuffers();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();
		void createCommandBuffers();

	public:
		Pipeline(SwapChain* swapChain);
		~Pipeline();

		VkCommandBuffer commandBuffer(size_t index);
		GraphicsBuffer* uniformBuffer(size_t index);

		void rebuild(SwapChain* swapChain);
	};
}
