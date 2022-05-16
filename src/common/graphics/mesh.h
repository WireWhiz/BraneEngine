#pragma once
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <array>
#include <vector>

#include "graphicsBuffer.h"
#include <common/ecs/core/component.h>
#include <assets/types/meshAsset.h>

namespace graphics
{

	struct MeshPushConstants
	{
		glm::mat4x4 render_matrix;
		glm::mat4x4 objectToWorld;
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

	public:


		Mesh(MeshAsset* meshAsset);
		~Mesh();

		MeshAsset* meshAsset();
		VkBuffer buffer() const;
		VkDeviceSize indexBufferOffset(uint32_t primitive) const;
        VkDeviceSize attributeBufferOffset(uint32_t primitive, const std::string& name) const;

		uint32_t size() const;
        uint32_t indexCount(uint32_t primitive) const;
		uint32_t vertexCount(uint32_t primitive) const;
		uint32_t primitiveCount() const;

		void updateData();

		void lock();
		void unlock();

	};
}