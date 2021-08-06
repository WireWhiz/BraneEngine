#include "graphics.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>

namespace graphics
{

    Window::Window()
    {
        init();
    }

    Window::~Window()
    {
        cleanup();
    }

    void Window::createSurface(VkInstance instance, const VkAllocationCallbacks* allocator)
    {
        if (glfwCreateWindowSurface(instance, _window, nullptr, &_surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void Window::destroySurface(VkInstance instance, const VkAllocationCallbacks* allocator)
    {
        vkDestroySurfaceKHR(instance, _surface, nullptr);
    }

    VkSurfaceKHR Window::surface()
    {
        return _surface;
    }

    void Window::init()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _window = glfwCreateWindow(800, 600, "Brane Surfer", nullptr, nullptr);
        assert(_window != NULL);


    }

    void Window::update()
    {
        glfwPollEvents();
        glfwGetFramebufferSize(_window, &_size.x, &_size.y);
    }

    bool Window::closed()
    {
        return glfwWindowShouldClose(_window);
    }

    glm::ivec2 Window::size()
    {
        return _size;
    }

    void Window::cleanup()
    {

        glfwDestroyWindow(_window);

        glfwTerminate();
    }
    GraphicsRuntime::GraphicsRuntime()
    {
        init();
    }
    GraphicsRuntime::~GraphicsRuntime()
    {
        cleanup();
    }
    void GraphicsRuntime::init()
	{
        _window = new Window();
        _vkRuntime = new VulkanRuntime(_window);
	}
    void GraphicsRuntime::update()
    {
        _window->update();
        _vkRuntime->draw();
    }
    bool GraphicsRuntime::windowClosed()
    {
        return _window->closed();
    }
    void GraphicsRuntime::cleanup()
    {
        delete _vkRuntime;
        delete _window;
        
    }

    void VulkanRuntime::draw()
    {
        vkWaitForFences(_device->logicalDevice(), 1, &_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = _pipeline->acquireNextImage(_imageAvailableSemaphores[currentFrame], imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            delete _pipeline;
            _pipeline = new GraphicsPipeline(_window, _device, _commandPool);
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        

        if (_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(_device->logicalDevice(), 1, &_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        _imagesInFlight[imageIndex] = _inFlightFences[currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = _pipeline->commandBuffer(imageIndex);

        VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(_device->logicalDevice(), 1, &_inFlightFences[currentFrame]);
        if (vkQueueSubmit(_device->graphicsQueue(), 1, &submitInfo, _inFlightFences[currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { _pipeline->swapchain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        vkQueuePresentKHR(_device->presentQueue(), &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanRuntime::init()
    {
        SpirvHelper::Init();
        createInstance();
        setupDebugMessenger();

        _window->createSurface(_instance, nullptr);
        _device = new GraphicsDevice(_instance, _window->surface());

        createCommandPool();

        _pipeline = new GraphicsPipeline(_window, _device, _commandPool);

        createSyncObjects();

    }

    void VulkanRuntime::cleanup()
    {
        vkDeviceWaitIdle(_device->logicalDevice());

        delete _pipeline;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(_device->logicalDevice(), _renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(_device->logicalDevice(), _imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(_device->logicalDevice(), _inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(_device->logicalDevice(), _commandPool, nullptr);

        delete _device;

        if (_validationLayersEnabled)
            DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);

        _window->destroySurface(_instance, nullptr);

        vkDestroyInstance(_instance, nullptr);

        SpirvHelper::Finalize();
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

        // get required extentions
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
        if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create vulkan instance!");
        }


    }

    

    void GraphicsPipeline::createSwapChain()
    {
        auto swapChainSupport = _device->swapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = _device->swapSurfaceFormat();
        VkPresentModeKHR presentMode = _device->swapPresentMode();
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = _window->surface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto indices = _device->queueFamilyIndices();
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        _swapChainImageFormat = surfaceFormat.format;
        _swapChainExtent = extent;

        if (vkCreateSwapchainKHR(_device->logicalDevice(), &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }
        vkGetSwapchainImagesKHR(_device->logicalDevice(), _swapChain, &imageCount, nullptr);
        _swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(_device->logicalDevice(), _swapChain, &imageCount, _swapChainImages.data());
    }

    void GraphicsPipeline::createImageViews()
    {
        _swapChainImageViews.resize(_swapChainImages.size());
        for (size_t i = 0; i < _swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = _swapChainImages[i];

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = _swapChainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_device->logicalDevice(), &createInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void GraphicsPipeline::createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = _swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;

        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;

        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(_device->logicalDevice(), &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void GraphicsPipeline::createGraphicsPipline()
    {
        VkShaderModule fragmentS; 
        VkShaderModule vertexS;
        std::thread t1 = asyncLoadShaderFromFile(VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/src/shaders.frag", &fragmentS);
        std::thread t2 = asyncLoadShaderFromFile(VK_SHADER_STAGE_VERTEX_BIT, "shaders/src/shaders.vert", &vertexS);

        t1.join();
        t2.join();

        _shaders.insert(std::make_pair("frag", fragmentS));
        _shaders.insert(std::make_pair("vert", vertexS));

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

        vertShaderStageInfo.module = vertexS;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragmentS;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getVkBindingDescription();
        auto attributeDescriptions = Vertex::getVkAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; 
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)_swapChainExtent.width;
        viewport.height = (float)_swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = _swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(_device->logicalDevice(), &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;

        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr; // Optional

        pipelineInfo.layout = _pipelineLayout;

        pipelineInfo.renderPass = _renderPass;
        pipelineInfo.subpass = 0;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(_device->logicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    void GraphicsPipeline::createFramebuffers()
    {
        _swapChainFramebuffers.resize(_swapChainImageViews.size());

        for (size_t i = 0; i < _swapChainImageViews.size(); i++)
        {
            VkImageView attachments[] = {
                _swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = _renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = _swapChainExtent.width;
            framebufferInfo.height = _swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(_device->logicalDevice(), &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void VulkanRuntime::createCommandPool()
    {
        auto queueFamilyIndices = _device->queueFamilyIndices();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = 0; // Optional

        if (vkCreateCommandPool(_device->logicalDevice(), &poolInfo, nullptr, &_commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void GraphicsPipeline:: createVertexBuffer()
    {
        _vertexBuffer = new GraphicsBuffer<Vertex>(_device, vertices.size());
        _vertexBuffer->setData(vertices, 0);
    }

    void GraphicsPipeline::createCommandBuffers(VkCommandPool commandPool)
    {
        _commandBuffers.resize(_swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

        if (vkAllocateCommandBuffers(_device->logicalDevice(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (size_t i = 0; i < _commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = _renderPass;
            renderPassInfo.framebuffer = _swapChainFramebuffers[i];

            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = _swapChainExtent;

            VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

            VkBuffer vertexBuffers[] = { _vertexBuffer->get() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);

            vkCmdDraw(_commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
            vkCmdEndRenderPass(_commandBuffers[i]);
            if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }

            
        }
    }

    void VulkanRuntime::createSyncObjects()
    {
        _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        _imagesInFlight.resize(_pipeline->swapChainLength(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(_device->logicalDevice(), &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(_device->logicalDevice(), &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(_device->logicalDevice(), &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
            {

                throw std::runtime_error("failed to create semaphores for a frame!");
            }
        }
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
            if (messageSeverity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                std::cerr << "Vulkan [Warning]: ";
            else if (messageSeverity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                std::cerr << "Vulkan [Error]: ";
            else
                std::cerr << "Vulkan [Unknown]: ";
            std::cerr  << pCallbackData->pMessage << std::endl;
        }
        return VK_FALSE;
    }

    

    VkExtent2D GraphicsPipeline::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            glm::vec2 size = _window->size();
            

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(size.x),
                static_cast<uint32_t>(size.y)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }


    

    void GraphicsPipeline::clearShaders()
    {
        for (auto& shader : _shaders)
            vkDestroyShaderModule(_device->logicalDevice(), shader.second, nullptr);
        _shaders.clear();
    }

    VulkanRuntime::VulkanRuntime(Window* window)
    {
        _window = window;
#ifdef DEBUG
        _validationLayersEnabled = true;
#else
        _validationLayersEnabled = false;
#endif
        init();
    }

    VulkanRuntime::~VulkanRuntime()
    {
        cleanup();
    }

    VkInstance VulkanRuntime::instance()
    {
        return _instance;
    }

    VkDevice VulkanRuntime::device()
    {
        return _device->logicalDevice();
    }

    VkPhysicalDevice VulkanRuntime::physicalDevice()
    {
        return _device->physicalDevice();
    }

    VkCommandPool VulkanRuntime::commandPool()
    {
        return _commandPool;
    }

    Window* VulkanRuntime::window()
    {
        return _window;
    }

    void GraphicsPipeline::loadShaderFromFile(VkDevice device, VkShaderStageFlagBits shaderType, const std::string& filename, VkShaderModule* shader)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("failed to open file!");

        // Make array of the same size as the file
        size_t fileSize = file.tellg();
        char* code = new char[fileSize + 1];
        code[fileSize] = '\0'; // turn this into a c str

        //read the array into the file
        file.seekg(0);
       
        file.read(code, fileSize);

        file.close();

        //convert file into shader
        std::vector<unsigned int> spirv;
        if (!CompileGLSL(shaderType, code, spirv))
            throw std::runtime_error("failed to compile shader!");

        delete[] code;

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = spirv.size() * 4;
        createInfo.pCode = spirv.data();

        if (vkCreateShaderModule(device, &createInfo, nullptr, shader) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to load shader!");
        }

        std::cout << "Compiled: " << filename << std::endl;
    }

    std::thread GraphicsPipeline::asyncLoadShaderFromFile(VkShaderStageFlagBits shaderType, const std::string& filename, VkShaderModule* shader)
    {
        std::thread t(loadShaderFromFile, _device->logicalDevice(), shaderType, filename, shader);
        return t;
    }

    GraphicsPipeline::GraphicsPipeline(Window* window, GraphicsDevice* device, VkCommandPool commandPool)
    {
        _window = window;
        _device = device;

        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipline();
        createFramebuffers();
        createVertexBuffer();
        createCommandBuffers(commandPool);

    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        vkDeviceWaitIdle(_device->logicalDevice());

        for (auto framebuffer : _swapChainFramebuffers)
        {
            vkDestroyFramebuffer(_device->logicalDevice(), framebuffer, nullptr);
        }
        
        delete _vertexBuffer;

        vkDestroyPipeline(_device->logicalDevice(), _graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(_device->logicalDevice(), _pipelineLayout, nullptr);
        vkDestroyRenderPass(_device->logicalDevice(), _renderPass, nullptr);

        clearShaders();

        for (auto imageView : _swapChainImageViews)
        {
            vkDestroyImageView(_device->logicalDevice(), imageView, nullptr);
        }

        vkDestroySwapchainKHR(_device->logicalDevice(), _swapChain, nullptr);

        
    }

    size_t GraphicsPipeline::swapChainLength()
    {
        return _swapChainImages.size();
    }

    VkResult GraphicsPipeline::acquireNextImage(VkSemaphore semaphore, uint32_t& index)
    {
        
        return vkAcquireNextImageKHR(_device->logicalDevice(), _swapChain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &index);
    }

    VkCommandBuffer* GraphicsPipeline::commandBuffer(size_t index)
    {
        assert(0 <= index && index < _commandBuffers.size());
        return &_commandBuffers[index];
    }

    VkSwapchainKHR GraphicsPipeline::swapchain()
    {
        return _swapChain;
    }
}