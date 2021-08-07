#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "graphicsDevice.h"

namespace graphics
{
    template<class T>
    class GraphicsBuffer
    {
        VkDevice _device;
	    VkBuffer _buffer;
	    VkDeviceMemory _memory;
        size_t _size;
    public:
	    GraphicsBuffer(GraphicsDevice* device, size_t size, VkBufferUsageFlags useage, VkMemoryPropertyFlags memFlags)
	    {
            assert(size > 0);
            _size = size;
            _device = device->logicalDevice();

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = sizeof(T) * size;
            bufferInfo.usage = useage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(_device, &bufferInfo, nullptr, &_buffer) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create vertex buffer!");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(_device, _buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, memFlags);

            if (vkAllocateMemory(_device, &allocInfo, nullptr, &_memory) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate vertex buffer memory!");
            }
            vkBindBufferMemory(_device, _buffer, _memory, 0);

            
	    }
	    ~GraphicsBuffer()
	    {
            vkDestroyBuffer(_device, _buffer, nullptr);
            vkFreeMemory(_device, _memory, nullptr);
	    }
        VkBuffer get()
        {
            return _buffer;
        }
        size_t size()
        {
            return _size;
        }
        void setData(std::vector<T> data, size_t startIndex)
        {
            assert(startIndex >= 0);
            assert(startIndex + data.size() <= _size);

            void* dataPtr;
            vkMapMemory(_device, _memory, startIndex, data.size(), 0, &dataPtr);
            memcpy(dataPtr, data.data(), data.size() * sizeof(T));
            vkUnmapMemory(_device, _memory);
        }
        void copy(GraphicsBuffer<T>* src, VkCommandBuffer commandBuffer)
        {
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0; // Optional
            copyRegion.dstOffset = 0; // Optional
            copyRegion.size = _size * sizeof(T);
            vkCmdCopyBuffer(commandBuffer, src->get(), _buffer, 1, &copyRegion);
        }
    };
}