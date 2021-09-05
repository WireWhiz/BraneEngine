#pragma once

#include "window.h"
#include "graphicsBuffer.h"
#include "validationLayers.h"
#include "material.h"
#include "renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <chrono>
#include "ecs/core/Entity.h"
#include "ecs/nativeTypes/transform.h"



namespace graphics
{

	class VulkanRuntime
	{
		Window* _window;
		GraphicsDevice* _device;
		SwapChain* _swapChain;
		ShaderManager* _shaderManager;
		std::vector<VkCommandBuffer> _drawBuffers;

		std::unordered_map<MaterialID, std::unique_ptr<Material>> _materials;
		std::unordered_map<MaterialID, std::unique_ptr<Renderer>> _renderers;
		std::unordered_map<TextureID, std::unique_ptr<Texture>> _textures;
		std::unordered_map<MeshID, std::unique_ptr<Mesh>> _meshes;

		VkInstance _instance;

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

		void createDrawBuffers();

		bool validationLayersSupported();
		void setDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void setupDebugMessenger();
		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

		
	public:
		
		VulkanRuntime();
		~VulkanRuntime();

		VkInstance instance();
		VkDevice device();
		VkPhysicalDevice physicalDevice();
		Window* window();

		void updateWindow();

		// Behold, the only function we care about:
		void draw(EntityManager& em);
		// Bask in it's glory!
		
		Texture* loadTexture(TextureID id);
		Shader* loadShader(ShaderID id);
		Material* createMaterial(MaterialID id);
		void initMaterial(EntityManager& em, MaterialID id);
		void addMesh(std::unique_ptr<Mesh> mesh, MeshID id);
		void updateUniformBuffer(EntityManager& em, ComponentID component);
		
	};

}