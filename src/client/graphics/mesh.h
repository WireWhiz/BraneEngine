#pragma once
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <array>
#include <vector>

#include "graphicsBuffer.h"
#include <common/ecs/core/Component.h>
#include <assets/types/meshAsset.h>

namespace graphics
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 uv;
		
		Vertex(glm::vec3 pos, glm::vec3 color, glm::vec2 uv)
		{
			this->pos = pos;
			this->color = color;
			this->uv = uv;
		}

		static VkVertexInputBindingDescription getVkBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getVkAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, uv);

			return attributeDescriptions;
		}
	};

	struct MeshPushConstants
	{
		glm::mat4x4 render_matrix;
	};

	typedef uint64_t MeshID;
	class Mesh
	{
	private:
		bool _locked;
		GraphicsBuffer* _stagingBuffer;
		GraphicsBuffer* _dataBuffer;
		MeshAsset* _meshAsset;

		std::vector<size_t> _primitiveBufferOffsets;

	public:


		Mesh(MeshAsset* meshAsset);
		~Mesh();

		VkBuffer indexBuffer(uint32_t primitive) const;
		VkDeviceSize Mesh::indexBufferOffset(uint32_t primitive) const;
		std::vector<VkBuffer> vertexBuffers(uint32_t primitive) const;
		std::vector<VkDeviceSize> vertexBufferOffsets(uint32_t primitive) const;

		uint32_t size() const;
		uint32_t vertexCount(uint32_t primitive) const;
		uint32_t primitiveCount() const;

		void lock();
		void unlock();

	};
}