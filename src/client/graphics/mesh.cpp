#include "mesh.h"

namespace graphics
{
	Mesh::Mesh(MeshAsset* meshAsset)
	{
		_meshAsset = meshAsset;
		_locked = true;
		unlock();


		_stagingBuffer->setData(_meshAsset->packedData(), 0);

		_dataBuffer = new GraphicsBuffer(size(),
											 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
											 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		SingleUseCommandBuffer cmdBuffer(device->transferPool());
		_dataBuffer->copy(_stagingBuffer, cmdBuffer.get(), size());
		cmdBuffer.submit(device->transferQueue());
	}
	Mesh::~Mesh()
	{
		if(!_locked)
				delete _stagingBuffer;
		delete _dataBuffer;

	}
	uint32_t Mesh::size()const
	{
		return _meshAsset->meshSize();
	}
	void Mesh::lock()
	{
		if (!_locked)
		{
			delete _stagingBuffer;
			_locked = false;
		}
	}
	void Mesh::unlock()
	{
		if (_locked)
		{
			_stagingBuffer = new GraphicsBuffer(size(),
												VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
												VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			_locked = false;
		}
	}

	uint32_t Mesh::vertexCount() const
	{
		return _meshAsset->indices.size();
	}

	VkBuffer Mesh::indexBuffer() const
	{
		return _dataBuffer->get();
	}

	std::vector<VkBuffer> Mesh::vertexBuffers() const
	{
		std::vector<VkBuffer> buffers(3 + _meshAsset->uvs.size());
		for (uint16_t i = 0; i < buffers.size(); ++i)
		{
			buffers[i] = _dataBuffer->get();
		}
		return buffers;
	}

	std::vector<VkDeviceSize> Mesh::vertexBufferOffsets() const
	{
		std::vector<VkDeviceSize> offsets;
		offsets.reserve(3 + _meshAsset->uvs.size());

		offsets.push_back(_meshAsset->indices.size() * sizeof(uint16_t));
		offsets.push_back(offsets[0] + _meshAsset->positions.size() * sizeof(glm::vec3));
		offsets.push_back(offsets[1] + _meshAsset->normals.size() * sizeof(glm::vec3));
		if(!_meshAsset->uvs.empty())
			offsets.push_back(offsets[2] + _meshAsset->tangents.size() * sizeof(glm::vec3));
		for (uint8_t i = 1; i < _meshAsset->uvs.size(); ++i)
		{
			offsets.push_back(offsets[2 + i] + _meshAsset->uvs[i-1].size() * sizeof(glm::vec2));
		}

		return offsets;
	}
}