#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include <iostream>
#include <assert.h>
#include <vector>
#include <cstring>
#include <map>
#include <set>
#include <optional>
#include <algorithm>
#include <fstream>

#include "shaders.h"

namespace graphics
{
	class Window
	{
	private:
		GLFWwindow* _window;
		glm::ivec2 _size;
		void init();
		void cleanup();
	public:
		Window();
		~Window();
		void createSurface(VkInstance instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);
		void update();
		bool closed();
		glm::ivec2 size();
		void setSize(glm::ivec2& newSize);
	};

	class VulkanRuntime
	{
		Window* _window;

		VkInstance _instance;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
		VkDevice _device;
		VkSurfaceKHR _surface;
		VkSwapchainKHR _swapChain;
		VkRenderPass _renderPass;
		VkPipelineLayout _pipelineLayout;
		VkPipeline _graphicsPipeline;
		VkCommandPool _commandPool;
		std::vector<VkCommandBuffer> _commandBuffers;

		VkQueue _graphicsQueue;
		VkQueue _presentQueue;

		std::vector<VkSemaphore> _imageAvailableSemaphores;
		std::vector<VkSemaphore> _renderFinishedSemaphores;

		std::vector<VkFramebuffer> _swapChainFramebuffers;
		std::vector<VkImage> _swapChainImages;
		std::vector<VkImageView> _swapChainImageViews;
		VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;

		std::unordered_map<std::string, VkShaderModule> _shaders;

		bool _validationLayersEnabled;
		VkDebugUtilsMessengerEXT _debugMessenger;
		const std::vector<const char*> _validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		const std::vector<const char*> _deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;
			bool isComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		const int MAX_FRAMES_IN_FLIGHT = 2;
		std::vector<VkFence> _inFlightFences;
		std::vector<VkFence> _imagesInFlight;
		size_t currentFrame = 0;

		void init();
		void cleanup();
		void createInstance();
		void getSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createGraphicsPipline();
		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createSyncObjects();

		bool validationLayersSupported();
		std::vector<const char*> requiredExtensions();

		bool deviceHasExtentionSupport(VkPhysicalDevice device);
		int deviceValue(VkPhysicalDevice device);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void clearShaders();

		void setDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void setupDebugMessenger();
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

		
	public:
		VulkanRuntime(Window* window);
		~VulkanRuntime();

		// Behold, the only function we care about:
		void draw();
		// Bask in it's glory!

		VkShaderModule loadShaderFromFile(VkShaderStageFlagBits shaderType, const std::string& filename);
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