#pragma once

#include "window.h"

#include <algorithm>
#include <vector>
#include <array>

namespace graphics
{
	class SwapChain
	{
		Window* _window;
		VkSwapchainKHR _swapChain;
		VkRenderPass _renderPass;
		VkRenderPass _imGuiRenderPass;

		std::vector<VkFramebuffer> _frameBuffers;
		std::vector<VkFramebuffer> _imGuiFrameBuffers;
		std::vector<VkImage> _images;
		std::vector<VkImageView> _imageViews;

		VkImage _depthImage;
		VkDeviceMemory _depthImageMemory;
		VkImageView _depthImageView;

		VkFormat _imageFormat;
		VkFormat _depthImageFormat;
		VkImageLayout _imageLayout;
		VkExtent2D _extent;
		static size_t _size;

		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createImGuiRenderPass();
		void createFrameBuffers();
		void createImGuiFrameBuffers();


		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findDepthFormat();

	public:
		SwapChain(Window* window);
		~SwapChain();

		VkSwapchainKHR get();
		VkResult acquireNextImage(VkSemaphore semaphore, uint32_t& index);
		VkFramebuffer framebuffer(size_t index);
		VkFramebuffer imGuiFramebuffer(size_t index);

		static size_t size();
		VkExtent2D extent();
		VkFormat imageFormat();
		VkFormat depthImageFormat();
		VkRenderPass renderPass();
		VkRenderPass imGuiRenderPass();
		VkImage getImage(size_t index);
	};
}