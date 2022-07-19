#include "graphics.h"
#include "imgui_internal.h"


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace graphics
{
    void VulkanRuntime::draw(EntityManager& em)
    {
		if(_window->size().x == 0 ||  _window->size().y == 0 || _renderers.empty())
		{
			return;
		}
        vkWaitForFences(_device->get(), 1, &_inFlightFences[_swapChain->nextFrame()], VK_TRUE, UINT64_MAX);

        VkResult result = _swapChain->acquireNextImage();
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            vkDeviceWaitIdle(_device->get());
            _swapChain->resize();
			for(auto& r : _renderers)
				if(r->targetingSwapChain())
					r->setTargetAsSwapChain(r->depthTexture());
			result = _swapChain->acquireNextImage();
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        

        if (_imagesInFlight[_swapChain->currentFrame()] != VK_NULL_HANDLE)
        {
            vkWaitForFences(_device->get(), 1, &_imagesInFlight[_swapChain->currentFrame()], VK_TRUE, UINT64_MAX);
        }
        _imagesInFlight[_swapChain->currentFrame()] = _inFlightFences[_swapChain->currentFrame()];

        VkCommandBuffer drawBuffer = _drawBuffers[_swapChain->currentFrame()];
        vkResetCommandBuffer(drawBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(drawBuffer, &beginInfo);

	    for (size_t i = _renderers.size() - 1; i < _renderers.size(); --i)
	    {
		    _renderers[i]->render(drawBuffer);
	    }

        vkEndCommandBuffer(drawBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { _swapChain->currentSemaphore() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        std::vector<VkCommandBuffer> cmdBuffers = {drawBuffer};
        submitInfo.commandBufferCount = cmdBuffers.size();
        submitInfo.pCommandBuffers = cmdBuffers.data();

        VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_swapChain->currentFrame()] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(_device->get(), 1, &_inFlightFences[_swapChain->currentFrame()]);
        if (vkQueueSubmit(_device->graphicsQueue(), 1, &submitInfo, _inFlightFences[_swapChain->currentFrame()]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { _swapChain->get() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &_swapChain->currentFrame();
        presentInfo.pResults = nullptr; // Optional
        vkQueuePresentKHR(_device->presentQueue(), &presentInfo);
    }

	size_t VulkanRuntime::addTexture(Texture* texture)
    {
        return _textures.push(std::unique_ptr<Texture>(texture));
    }
	Shader* VulkanRuntime::loadShader(size_t shaderID)
    {
        return _shaderManager->loadShader(shaderID);
    }
    size_t VulkanRuntime::addMesh(MeshAsset* mesh)
    {
		uint32_t index = _meshes.size();
		mesh->pipelineID = index;
        _meshes.push(std::make_unique<Mesh>(mesh));
		return index;
    }

    void VulkanRuntime::init()
    {

        createInstance();
        setupDebugMessenger();

        _window->createSurface(_instance, nullptr);
        _device = new GraphicsDevice(_instance, _window->surface());
        _swapChain = new SwapChain(_window);
        _shaderManager = new ShaderManager();

        createSyncObjects();
        createDrawBuffers();


    }

    void VulkanRuntime::cleanup()
    {
        vkDeviceWaitIdle(_device->get());

        _renderers.clear();
        _materials.clear();
        _textures.clear();
        _meshes.clear();
        
        delete _shaderManager;
        delete _swapChain;
        
        for (size_t i = 0; i < _renderFinishedSemaphores.size(); i++)
        {
            vkDestroySemaphore(_device->get(), _renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(_device->get(), _inFlightFences[i], nullptr);
        }

        delete _device;

        if (_validationLayersEnabled)
            DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);

        _window->destroySurface(_instance, nullptr);

        vkDestroyInstance(_instance, nullptr);
    }

    void VulkanRuntime::createInstance()
    {
        if (_validationLayersEnabled && !validationLayersSupported())
        {
            throw std::runtime_error("vulkan validation layers are not available!");
        }

        // Set up app info
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Brane Surfer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Brane Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        //

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // get required extensions
        std::vector<const char*> extensions = requiredExtensions();

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        //

        // validation layers and debug callback
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (_validationLayersEnabled)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            setDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }
        //

        //Create Instance
        VkResult res = vkCreateInstance(&createInfo, nullptr, &_instance);
        if (res != VK_SUCCESS)
        {
            Runtime::error("Could not create vulkan instance: " + std::to_string(res));
            throw std::runtime_error("failed to create vulkan instance!");
        }
        Runtime::log("Created vulkan instance");
    }

    void VulkanRuntime::createSyncObjects()
    {
        _renderFinishedSemaphores.resize(_swapChain->size());
        _inFlightFences.resize(_swapChain->size());
        _imagesInFlight.resize(_swapChain->size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < _swapChain->size(); i++)
        {
            if (vkCreateSemaphore(_device->get(), &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(_device->get(), &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
            {

                throw std::runtime_error("failed to create semaphores for a frame!");
            }
        }
    }

    void VulkanRuntime::createDrawBuffers()
    {
        _drawBuffers.resize(_swapChain->size());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = _device->graphicsPool();
        allocInfo.commandBufferCount = _swapChain->size();

        vkAllocateCommandBuffers(_device->get(), &allocInfo, _drawBuffers.data());
    }


    bool VulkanRuntime::validationLayersSupported()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    

    std::vector<const char*> VulkanRuntime::requiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (_validationLayersEnabled)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void VulkanRuntime::setDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void VulkanRuntime::setupDebugMessenger()
    {
        if (!_validationLayersEnabled)
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        setDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(_instance,&createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }

    }

    VkResult VulkanRuntime::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void VulkanRuntime::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRuntime::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (messageSeverity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                Runtime::warn("Vulkan: " + std::string(pCallbackData->pMessage));
            else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	            Runtime::error("Vulkan: " + std::string(pCallbackData->pMessage));
            else
                Runtime::log("Vulkan: "  + std::string(pCallbackData->pMessage));
#ifndef NDEBUG
			if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
				throw std::runtime_error("Vulkan debug error");
#endif
        }
        return VK_FALSE;
    }

    

    

    VulkanRuntime::VulkanRuntime()
    {
        _window = new Window();
#ifndef NDEBUG
        _validationLayersEnabled = true;
#else
        _validationLayersEnabled = false;
#endif
        init();
	    EntityManager* em = Runtime::getModule<EntityManager>();

		Runtime::timeline().addTask("draw", [this, em]{
			updateWindow();
			if(_window->closed())
			{

				Runtime::stop();
				return;
			}
			draw(*em);
		}, "draw");
    }

    VulkanRuntime::~VulkanRuntime()
    {
        cleanup();
        delete _window;
    }

    VkInstance VulkanRuntime::instance()
    {
        return _instance;
    }

    VkDevice VulkanRuntime::device()
    {
        return _device->get();
    }

    VkPhysicalDevice VulkanRuntime::physicalDevice()
    {
        return _device->physicalDevice();
    }

    Window* VulkanRuntime::window()
    {
        return _window;
    }

    void VulkanRuntime::updateWindow()
    {
        _window->update();
    }

	void VulkanRuntime::addMaterial(Material* material)
	{
		material->buildPipelineLayout(_swapChain);
		material->initialize(_swapChain->size());
		_materials.push(std::unique_ptr<Material>(material));
	}

	Material* VulkanRuntime::getMaterial(size_t id)
	{
		return _materials[id].get();
	}

	const char* VulkanRuntime::name()
	{
		return "graphics";
	}


	SwapChain* VulkanRuntime::swapChain()
	{
		return _swapChain;
	}

	const staticIndexVector<std::unique_ptr<Material>>& VulkanRuntime::materials()
	{
		return _materials;
	}

	const staticIndexVector<std::unique_ptr<Mesh>>& VulkanRuntime::meshes()
	{
		return _meshes;
	}


}