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

	struct MeshPushConstants
	{
		glm::mat4x4 render_matrix;
		glm::vec4 lightPosition;
	};

	typedef uint64_t MeshID;
	class Mesh
	{
	private:
		bool _locked;
		GraphicsBuffer* _stagingBuffer;
		GraphicsBuffer* _dataBuffer;
		MeshAsset* _meshAsset;

		std::vector<std::vector<VkDeviceSize>> _primitiveBufferOffsets;

	public:


		Mesh(MeshAsset* meshAsset);
		~Mesh();

		const MeshAsset* meshAsset();
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