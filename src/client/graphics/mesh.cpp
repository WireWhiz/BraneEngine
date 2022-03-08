#include "mesh.h"

namespace graphics
{
	Mesh::Mesh(MeshAsset* meshAsset)
	{
		_meshAsset = meshAsset;
		_locked = true;
		unlock();


		size_t offset = 0;
		_primitiveBufferOffsets.reserve(meshAsset->primitives.size());
		for(auto& p : meshAsset->primitives)
		{
			std::vector<byte> data = p.packedData();
			_stagingBuffer->setData(data, offset);
			_primitiveBufferOffsets.push_back(offset);
			offset += p.meshSize();
		}


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

	uint32_t Mesh::vertexCount(uint32_t primitive) const
	{
		return _meshAsset->primitives[primitive].indices.size();
	}

	VkBuffer Mesh::indexBuffer(uint32_t primitive) const
	{
		return _dataBuffer->get();
	}

	VkDeviceSize Mesh::indexBufferOffset(uint32_t primitive) const
	{
		return _primitiveBufferOffsets[primitive];
	}

	std::vector<VkBuffer> Mesh::vertexBuffers(uint32_t primitive) const
	{
		std::vector<VkBuffer> buffers(3 + _meshAsset->primitives[primitive].uvs.size());
		for (uint16_t i = 0; i < buffers.size(); ++i)
		{
			buffers[i] = _dataBuffer->get();
		}
		return buffers;
	}

	std::vector<VkDeviceSize> Mesh::vertexBufferOffsets(uint32_t primitive) const
	{
		std::vector<VkDeviceSize> offsets;
		offsets.reserve(3 + _meshAsset->primitives[primitive].uvs.size());

		offsets.push_back(_meshAsset->primitives[primitive].indices.size() * sizeof(uint16_t));
		offsets.push_back(offsets[0] + _meshAsset->primitives[primitive].positions.size() * sizeof(glm::vec3));
		offsets.push_back(offsets[1] + _meshAsset->primitives[primitive].normals.size() * sizeof(glm::vec3));
		if(!_meshAsset->primitives[primitive].uvs.empty())
			offsets.push_back(offsets[2] + _meshAsset->primitives[primitive].tangents.size() * sizeof(glm::vec3));

		for (uint8_t i = 1; i < _meshAsset->primitives[primitive].uvs.size(); ++i)
		{
			offsets.push_back(offsets[2 + i] + _meshAsset->primitives[primitive].uvs[i-1].size() * sizeof(glm::vec2));
		}


		for (uint16_t i = 0; i < offsets.size(); ++i)
		{
			offsets[i] += _primitiveBufferOffsets[primitive];
		}
		return offsets;
	}

	uint32_t Mesh::primitiveCount() const
	{
		return _meshAsset->primitives.size();
	}
}