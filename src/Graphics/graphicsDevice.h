#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <optional>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include <string>

#include "validationLayers.h"

namespace graphics
{

	class GraphicsDevice
	{

		VkDevice _device;
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;

		VkQueue _graphicsQueue;
		VkQueue _presentQueue;
		VkQueue _transferQueue;

		const std::vector<const char*> _deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	public:
		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;
			std::optional<uint32_t> transferFamily;
			bool isComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
			}
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};
	private:
		bool _validationLayersEnabled;
		QueueFamilyIndices _queueFamilyIndices;
		SwapChainSupportDetails _swapChainSupport;
		VkSurfaceFormatKHR _swapSurfaceFormat;
		VkPresentModeKHR _swapPresentMode;


		void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
		void createLogicalDevice();

		bool deviceHasExtentionSupport(VkPhysicalDevice device);
		int deviceValue(VkPhysicalDevice device, VkSurfaceKHR surface);

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	public:
		GraphicsDevice(VkInstance instance, VkSurfaceKHR surface);
		~GraphicsDevice();

		VkDevice logicalDevice();
		VkPhysicalDevice physicalDevice();

		SwapChainSupportDetails swapChainSupport(VkSurfaceKHR surface);
		VkSurfaceFormatKHR swapSurfaceFormat();
		VkPresentModeKHR swapPresentMode();
		QueueFamilyIndices queueFamilyIndices();
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkQueue graphicsQueue();
		VkQueue presentQueue();
		VkQueue transferQueue();
	};
}