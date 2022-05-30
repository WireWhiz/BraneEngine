#pragma once
#include <ecs/core/component.h>
#include <ecs/core/structMembers.h>
#include <ecs/nativeTypes/transform.h>
#include <ecs/nativeTypes/meshRenderer.h>
#include "mesh.h"
#include "material.h"
#include "RenderTarget.h"
#include "Camera.h"

namespace graphics
{

	typedef uint32_t RendererID;

	struct RenderObject
	{
		Mesh* mesh;
		size_t primitive;
		glm::mat4x4 transform;
	};

	class Renderer
	{
	protected:
		VkRenderPass _renderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> _frameBuffers;
		SwapChain& _swapChain;
		RenderTexture* _target = nullptr;

		std::function<void()> _onDestroy;
		VkClearColorValue _clearColor;
		VkExtent2D _extent = {0,0};
		bool _depthTexture = false;
		bool _shouldClear = true;

		void createRenderPass(VkFormat imageFormat, VkFormat depthImageFormat = VkFormat::VK_FORMAT_UNDEFINED);
		void createFrameBuffers(VkExtent2D size, const std::vector<VkImageView>& images, VkImageView depthTexture = VK_NULL_HANDLE);
		void startRenderPass(VkCommandBuffer cmdBuffer);
		void endRenderPass(VkCommandBuffer cmdBuffer);
		virtual void rebuild();
	public:
		uint8_t priority;
		Renderer(SwapChain& swapChain);
		virtual ~Renderer();

		bool targetingSwapChain();
		void setTargetAsSwapChain(bool depthTexture);
		void setTarget(RenderTexture* texture);
		void setClearColor(VkClearColorValue color);
		void dontClear();

		VkFramebuffer framebuffer(size_t index);
		VkExtent2D extent();


		virtual void render(VkCommandBuffer cmdBuffer) = 0;

		VkRenderPass renderPass();

		bool depthTexture();
		void clearTarget();
	};

	class CustomRenderer : public Renderer
	{
		std::function<void(VkCommandBuffer cmdBuffer)> _renderCallback;
	public:
		CustomRenderer(SwapChain& swapChain);
		void setRenderCallback(const std::function<void(VkCommandBuffer cmdBuffer)>& callback);
		void render(VkCommandBuffer cmdBuffer) override;
	};
}