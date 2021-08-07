#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <iostream>
#include <assert.h>
#include <vector>
#include <array>
#include <cstring>
#include <map>
#include <set>
#include <optional>
#include <algorithm>
#include <fstream>
#include <thread>

#include "shaders.h"
#include "graphicsBuffer.h"
#include "graphicsDevice.h"
#include "validationLayers.h"

namespace graphics
{

	

	class Window
	{
	private:
		GLFWwindow* _window;
		glm::ivec2 _size;
		VkSurfaceKHR _surface;
		void init();
		void cleanup();
	public:
		Window();
		~Window();
		void createSurface(VkInstance instance, const VkAllocationCallbacks* allocator);
		void destroySurface(VkInstance instance, const VkAllocationCallbacks* allocator);
		VkSurfaceKHR surface();
		void update();
		bool closed();
		glm::ivec2 size();
		void setSize(glm::ivec2& newSize);
	};

	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;

		static VkVertexInputBindingDescription getVkBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getVkAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}
	};

	const std::vector<Vertex> vertices = {
	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};


	class GraphicsPipeline
	{
		Window* _window;
		GraphicsDevice* _device;

		VkSwapchainKHR _swapChain;
		VkRenderPass _renderPass;
		VkPipelineLayout _pipelineLayout;
		VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;
		GraphicsBuffer<Vertex>* _vertexBuffer;
		GraphicsBuffer<Vertex>* _vertexStagingBuffer;

		std::vector<VkFramebuffer> _swapChainFramebuffers;
		std::vector<VkImage> _swapChainImages;
		std::vector<VkImageView> _swapChainImageViews;
		std::vector<VkCommandBuffer> _commandBuffers;

		VkPipeline _graphicsPipeline;


		std::unordered_map<std::string, VkShaderModule> _shaders;
		
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createGraphicsPipline();
		void createFramebuffers();
		void createVertexBuffer(VkCommandPool commandPool);
		void createCommandBuffers(VkCommandPool commandPool);

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void clearShaders();
		std::thread asyncLoadShaderFromFile(VkShaderStageFlagBits shaderType, const std::string& filename, VkShaderModule* shader);
		static void loadShaderFromFile(VkDevice device, VkShaderStageFlagBits shaderType, const std::string& filename, VkShaderModule* shader);

	public:
		GraphicsPipeline(Window* window, GraphicsDevice* device, VkCommandPool vertexCommandPool, VkCommandPool graphicsCommandPool);
		~GraphicsPipeline();

		size_t swapChainLength();
		VkResult acquireNextImage(VkSemaphore semaphore, uint32_t& index);
		VkCommandBuffer* commandBuffer(size_t index);
		VkSwapchainKHR swapchain();
	};

	

	class VulkanRuntime
	{
		Window* _window;
		GraphicsDevice* _device;
		GraphicsPipeline* _pipeline;

		VkInstance _instance;
		VkCommandPool _graphicsCommandPool;
		VkCommandPool _transferCommandPool;

		std::vector<VkSemaphore> _imageAvailableSemaphores;
		std::vector<VkSemaphore> _renderFinishedSemaphores;

		bool _validationLayersEnabled;
		VkDebugUtilsMessengerEXT _debugMessenger;

		std::vector<const char*> requiredExtensions();
		

		const int MAX_FRAMES_IN_FLIGHT = 2;
		std::vector<VkFence> _inFlightFences;
		std::vector<VkFence> _imagesInFlight;
		size_t currentFrame = 0;

		void init();
		void cleanup();
		void createInstance();
		void createCommandPools();
		void createSyncObjects();

		bool validationLayersSupported();
		void setDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void setupDebugMessenger();
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

		
	public:
		
		VulkanRuntime(Window* window);
		~VulkanRuntime();

		VkInstance instance();
		VkDevice device();
		VkPhysicalDevice physicalDevice();
		VkCommandPool commandPool();
		Window* window();


		// Behold, the only function we care about:
		void draw();
		// Bask in it's glory!
		
	};

	class GraphicsRuntime
	{
	private:
		Window* _window;
		VulkanRuntime* _vkRuntime;
		void init();
		void cleanup();
	public:
		GraphicsRuntime();
		~GraphicsRuntime();
		void update();
		bool windowClosed();
		
	};
}