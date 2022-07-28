//
// Created by wirewhiz on 28/07/22.
//

#include "graphicsBuffer.h"

namespace graphics
{

    SingleUseCommandBuffer::SingleUseCommandBuffer(VkCommandPool pool)
    {
        _pool = pool;
        _submitted = false;

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = pool;
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(device->get(), &allocInfo, &_buffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(_buffer, &beginInfo);
    }

    SingleUseCommandBuffer::~SingleUseCommandBuffer()
    {
        if(!_submitted)
            vkFreeCommandBuffers(device->get(), _pool, 1, &_buffer);
    }

    VkCommandBuffer SingleUseCommandBuffer::get()
    {
        assert(!_submitted && "Command buffer can not be accessed after it is submitted");
        return _buffer;
    }

    void SingleUseCommandBuffer::submit(VkQueue submitQueue)
    {
        vkEndCommandBuffer(_buffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_buffer;

        vkQueueSubmit(submitQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(submitQueue);

        vkFreeCommandBuffers(device->get(), _pool, 1, &_buffer);
        _submitted = true;
    }

    GraphicsBuffer::GraphicsBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memFlags)
    {
        assert(size > 0);
        _size = size;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device->get(), &bufferInfo, nullptr, &_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device->get(), _buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, memFlags);

        if (vkAllocateMemory(device->get(), &allocInfo, nullptr, &_memory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }
        vkBindBufferMemory(device->get(), _buffer, _memory, 0);


    }

    GraphicsBuffer::~GraphicsBuffer()
    {
        vkDestroyBuffer(device->get(), _buffer, nullptr);
        vkFreeMemory(device->get(), _memory, nullptr);
    }

    VkBuffer GraphicsBuffer::get() const
    {
        return _buffer;
    }

    size_t GraphicsBuffer::size() const
    {
        return _size;
    }

    void GraphicsBuffer::setData(const void* data, VkDeviceSize size, VkDeviceSize startIndex)
    {
        assert(startIndex >= 0);
        assert(startIndex + size <= _size);

        void* dataPtr;
        vkMapMemory(device->get(), _memory, startIndex, size, 0, &dataPtr);
        memcpy(dataPtr, data, size);
        vkUnmapMemory(device->get(), _memory);
    }

    void GraphicsBuffer::copy(GraphicsBuffer* src, VkCommandBuffer commandBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset)
    {
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = srcOffset;
        copyRegion.dstOffset = dstOffset;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, src->get(), _buffer, 1, &copyRegion);
    }
}