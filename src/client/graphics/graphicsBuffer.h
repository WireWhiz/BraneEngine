#pragma once

#include <GLFW/glfw3.h>
#include <cassert>
#include <cstring>

#include "graphicsDevice.h"

namespace graphics
{
    class SingleUseCommandBuffer
    {
        VkCommandBuffer _buffer;
        VkCommandPool _pool;
        bool _submitted;
    public:
        SingleUseCommandBuffer( VkCommandPool pool)
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
        ~SingleUseCommandBuffer()
        {
            if(!_submitted)
                vkFreeCommandBuffers(device->get(), _pool, 1, &_buffer);
        }
        VkCommandBuffer get()
        {
            assert(!_submitted && "Command buffer can not be accessed after it is submitted");
            return _buffer;
        }
        void submit(VkQueue submitQueue)
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
    };
    class GraphicsBuffer
    {
	    VkBuffer _buffer;
	    VkDeviceMemory _memory;
        size_t _size;
    public:
	    GraphicsBuffer(VkDeviceSize size, VkBufferUsageFlags useage, VkMemoryPropertyFlags memFlags)
	    {
            assert(size > 0);
            _size = size;

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = useage;
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
	    ~GraphicsBuffer()
	    {
            vkDestroyBuffer(device->get(), _buffer, nullptr);
            vkFreeMemory(device->get(), _memory, nullptr);
	    }
        VkBuffer get()
        {
            return _buffer;
        }
        size_t size()
        {
            return _size;
        }
        template<class T>
        void setData(const T& data, VkDeviceSize startIndex = 0)
        {
            assert(startIndex >= 0);
            assert(startIndex + sizeof(T) <= _size * sizeof(T));

            void* dataPtr;
            vkMapMemory(device->get(), _memory, startIndex, sizeof(T), 0, &dataPtr);
            memcpy(dataPtr, &data, sizeof(T));
            vkUnmapMemory(device->get(), _memory);
        }
        void setData(const void* data, VkDeviceSize size, VkDeviceSize startIndex)
        {
            assert(startIndex >= 0);
            assert(startIndex + size <= _size);

            void* dataPtr;
            vkMapMemory(device->get(), _memory, startIndex, size, 0, &dataPtr);
            memcpy(dataPtr, data, size);
            vkUnmapMemory(device->get(), _memory);
        }
        template <class T>
        void setData(const std::vector<T>& data, VkDeviceSize startIndex)
        {
            assert(startIndex >= 0);
            assert(startIndex + data.size() * sizeof(T) <= _size);

            void* dataPtr;
            vkMapMemory(device->get(), _memory, startIndex, data.size() * sizeof(T), 0, &dataPtr);
            memcpy(dataPtr, data.data(), data.size() * sizeof(T));
            vkUnmapMemory(device->get(), _memory);
        }
        void copy(GraphicsBuffer* src, VkCommandBuffer commandBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0)
        {
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = srcOffset; 
            copyRegion.dstOffset = dstOffset; 
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, src->get(), _buffer, 1, &copyRegion);
        }
    };
}