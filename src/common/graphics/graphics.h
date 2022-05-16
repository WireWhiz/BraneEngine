#pragma once

#include "window.h"
#include "graphicsBuffer.h"
#include "validationLayers.h"
#include "material.h"
#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <chrono>
#include "ecs/core/entity.h"
#include "ecs/nativeTypes/transform.h"
#include <utility/staticIndexVector.h>
#include <runtime/module.h>
#include <runtime/runtime.h>

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

namespace graphics
{

	class VulkanRuntime : public Module
	{
		Window* _window;
		GraphicsDevice* _device;
		SwapChain* _swapChain;
		ShaderManager* _shaderManager;
		std::vector<VkCommandBuffer> _drawBuffers;

		staticIndexVector<std::unique_ptr<Material>> _materials;
		staticIndexVector<std::unique_ptr<Renderer>> _renderers;
		staticIndexVector<std::unique_ptr<Texture>> _textures;
		staticIndexVector<std::unique_ptr<Mesh>> _meshes;

		VkInstance _instance;

		VkDescriptorPool _imGuiDescriptorPool;

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
		void createSyncObjects();
		void setupImGui();
		void cleanupImGui();

		void createDrawBuffers();

		bool validationLayersSupported();
		void setDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void setupDebugMessenger();
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

		
	public:
		
		VulkanRuntime(Runtime& runtime);
		~VulkanRuntime();

		const char* name() override;

		VkInstance instance();
		VkDevice device();
		VkPhysicalDevice physicalDevice();
		Window* window();

		void updateWindow();

		// Behold, the only function we care about:
		void draw(EntityManager& em);
		// Bask in its glory!
		
		size_t addTexture(Texture* texture);
		Shader* loadShader(size_t shaderID);
		size_t addMaterial(Material* material);
		size_t initRenderer(size_t id);
		size_t addMesh(MeshAsset* mesh);

		Material* getMaterial(size_t id);
	};

}