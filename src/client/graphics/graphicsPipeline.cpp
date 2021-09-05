#include "graphicsPipeline.h"

#include "graphicsDevice.h"

/*
namespace graphics
{

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    void Pipeline::createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;

        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device->get(), &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        
    }

    void Pipeline::createGraphicsPipline()
    {
        
    }

    void Pipeline::createTextureImage()
    {
        _texture = new Texture("textures/cropped-circut.png");
    }


    void Pipeline::createModelBuffers()
    {
        GraphicsBuffer stagingBuffer(Mesh::quad.vertices.size() * sizeof(Vertex) + Mesh::quad.indices.size() * sizeof(uint32_t),
                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.setData(Mesh::quad.indices, 0);
        stagingBuffer.setData(Mesh::quad.vertices, Mesh::quad.indices.size() * sizeof(uint32_t));

        _graphicsBuffer = new GraphicsBuffer(Mesh::quad.vertices.size() * sizeof(Vertex) + Mesh::quad.indices.size() * sizeof(uint32_t),
                                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        Stopwatch sw;
        SingleUseCommandBuffer cmdBuffer(device->transferPool());
        _graphicsBuffer->copy(&stagingBuffer, cmdBuffer.get(), Mesh::quad.vertices.size() * sizeof(Vertex) + Mesh::quad.indices.size() * sizeof(uint32_t));
        auto bufferTime = sw.time();
        std::cout << "Testing buffer creation and submission time" << std::endl;
        cmdBuffer.submit(device->transferQueue());
    }

    void Pipeline::createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        _uniformBuffers.reserve(_swapChain->size());

        for (size_t i = 0; i < _swapChain->size(); i++)
        {
            _uniformBuffers.emplace_back(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }

    void Pipeline::createDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(_swapChain->size());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(_swapChain->size());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();

        poolInfo.maxSets = static_cast<uint32_t>(_swapChain->size());

        if (vkCreateDescriptorPool(device->get(), &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void Pipeline::createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(_swapChain->size(), _descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(_swapChain->size());
        allocInfo.pSetLayouts = layouts.data();

        _descriptorSets.resize(_swapChain->size());
        if (vkAllocateDescriptorSets(device->get(), &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < _swapChain->size(); i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = _uniformBuffers[i].get();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = _texture->view();
            imageInfo.sampler = _sampler->get();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = _descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = _descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device->get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }


    }

    void Pipeline::createCommandBuffers()
    {
        _commandBuffers.resize(_swapChain->size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = device->graphicsPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

        if (vkAllocateCommandBuffers(device->get(), &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
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
            renderPassInfo.renderPass = _swapChain->renderPass();
            renderPassInfo.framebuffer = _swapChain->framebuffer(i);

            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = _swapChain->extent();

            VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

            vkCmdBindIndexBuffer(_commandBuffers[i], _graphicsBuffer->get(), 0, VK_INDEX_TYPE_UINT32);

            VkBuffer vertexBuffers[] = { _graphicsBuffer->get() };
            VkDeviceSize offsets[] = { static_cast<uint32_t>(Mesh::quad.indices.size() * sizeof(uint32_t)) };
            vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, vertexBuffers, offsets);

            vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[i], 0, nullptr);

            vkCmdDrawIndexed(_commandBuffers[i], static_cast<uint32_t>(Mesh::quad.indices.size()), 1, 0, 0, 0);

            vkCmdEndRenderPass(_commandBuffers[i]);

            if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }


        }
    }

    Pipeline::Pipeline(SwapChain* swapChain)
    {
        _swapChain = swapChain;

        createDescriptorSetLayout();
        createGraphicsPipline();
        createTextureImage();
        createModelBuffers();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();

    }

    Pipeline::~Pipeline()
    {
        delete _graphicsBuffer;
        delete _sampler;
        delete _texture;

        vkDestroyPipeline(device->get(), _pipeline, nullptr);
        vkDestroyPipelineLayout(device->get(), _pipelineLayout, nullptr);

        vkDestroyDescriptorSetLayout(device->get(), _descriptorSetLayout, nullptr);

        _uniformBuffers.clear(); // must be empty before pool is destroyed

        vkDestroyDescriptorPool(device->get(), _descriptorPool, nullptr);
    }

    

    VkCommandBuffer Pipeline::commandBuffer(size_t index)
    {
        assert(0 <= index && index < _commandBuffers.size());
        return _commandBuffers[index];
    }

    GraphicsBuffer* Pipeline::uniformBuffer(size_t index)
    {
        assert(index >= 0);
        assert(index < _uniformBuffers.size());
        return &_uniformBuffers[index];
    }
    void Pipeline::rebuild(SwapChain* swapChain)
    {
        _swapChain = swapChain;

        vkDestroyPipeline(device->get(), _pipeline, nullptr);
        vkDestroyPipelineLayout(device->get(), _pipelineLayout, nullptr);
        
        createGraphicsPipline();
        createCommandBuffers();
    }
}
*/