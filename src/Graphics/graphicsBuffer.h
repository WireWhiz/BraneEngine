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
	    bool _staged;
        size_t _size;
    public:
	    GraphicsBuffer(GraphicsDevice* device, size_t size, bool staged = false)
	    {
            _size = size;
            _staged = staged;
            _device = device->logicalDevice();

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = sizeof(T) * size;
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
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
            allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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
        void setData(std::vector<T> data, size_t startIndex)
        {
            assert(startIndex >= 0);
            assert(startIndex + data.size() <= _size);

            void* dataPtr;
            vkMapMemory(_device, _memory, 0, data.size(), 0, &dataPtr);
            memcpy(dataPtr, data.data(), data.size() * sizeof(T));
            vkUnmapMemory(_device, _memory);
        }
    };
}