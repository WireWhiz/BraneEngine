#pragma once
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <array>
#include <vector>

#include "graphicsBuffer.h"
#include <core/Component.h>

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
		uint32_t _size;
	public:
		std::vector<uint32_t> indices;
		std::vector<Vertex> vertices; 

		Mesh(std::vector<uint32_t> indices, std::vector<Vertex> vertices);
		~Mesh();

		VkBuffer data() const;
		size_t vertexBufferOffset() const;
		uint32_t size() const;
		void lock();
		void unlock();

	};

	class MeshComponent : public NativeComponent<MeshComponent, 1>
	{
	public:
		void getVariableIndicies(std::vector<NativeVarDef>& variables);
		MeshID id;
	};
	
}