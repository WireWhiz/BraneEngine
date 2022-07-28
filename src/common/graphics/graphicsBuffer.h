#pragma once

#include <GLFW/glfw3.h>
#include <cassert>
#include <cstring>
#include <vector>
#include "graphicsDevice.h"

namespace graphics
{
    class SingleUseCommandBuffer
    {
        VkCommandBuffer _buffer;
        VkCommandPool _pool;
        bool _submitted;
    public:
        SingleUseCommandBuffer(VkCommandPool pool);
        ~SingleUseCommandBuffer();
        VkCommandBuffer get();
        void submit(VkQueue submitQueue);
    };

    class GraphicsBuffer
    {
	    VkBuffer _buffer{};
	    VkDeviceMemory _memory{};
        size_t _size;
    public:
	    GraphicsBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memFlags);
	    ~GraphicsBuffer();
        VkBuffer get() const;
        size_t size() const;
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
        void setData(const void* data, VkDeviceSize size, VkDeviceSize startIndex);
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
        void copy(GraphicsBuffer* src, VkCommandBuffer commandBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);
    };
}