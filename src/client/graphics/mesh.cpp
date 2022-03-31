#include "mesh.h"

namespace graphics
{
	Mesh::Mesh(MeshAsset* meshAsset)
	{
		_meshAsset = meshAsset;
		_locked = true;
		unlock();

		size_t offset = 0;
		_primitiveBufferOffsets.resize(meshAsset->primitives.size());
		for (size_t i = 0; i < meshAsset->primitives.size(); ++i)
		{
			auto& p = _meshAsset->primitives[i];
			std::vector<byte> data = p.packedData();
			_stagingBuffer->setData(data, offset);


			_primitiveBufferOffsets[i].push_back(offset);

			offset += p.indices.size() * sizeof(uint16_t);
			if(offset % 4 != 0)
				offset += 2;
			_primitiveBufferOffsets[i].push_back(offset);

			offset += p.positions.size() * sizeof(glm::vec3);
			if(!p.positions.empty() && (!p.tangents.empty() || !p.uvs.empty() || !p.normals.empty()))
				_primitiveBufferOffsets[i].push_back(offset);

			offset += p.normals.size()  * sizeof(glm::vec3);
			if(!p.normals.empty() && (!p.tangents.empty() || !p.uvs.empty()))
				_primitiveBufferOffsets[i].push_back(offset);

			offset += p.tangents.size()  * sizeof(glm::vec3);
			if(!p.tangents.empty() && !p.uvs.empty())
				_primitiveBufferOffsets[i].push_back(offset);

			for (uint8_t j = 0; j < p.uvs.size(); ++j)
			{
				offset += p.uvs[j].size()  * sizeof(glm::vec2);
				if(j != p.uvs.size() - 1)
					_primitiveBufferOffsets[i].push_back(offset);
			}
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
		return _primitiveBufferOffsets[primitive][0];
	}

	std::vector<VkBuffer> Mesh::vertexBuffers(uint32_t primitive) const
	{
		std::vector<VkBuffer> buffers(_primitiveBufferOffsets[primitive].size() - 1);
		for (uint16_t i = 0; i < buffers.size(); ++i)
		{
			buffers[i] = _dataBuffer->get();
		}
		return buffers;
	}

	std::vector<VkDeviceSize> Mesh::vertexBufferOffsets(uint32_t primitive) const
	{
		std::vector<VkDeviceSize> offsets;
		for (int i = 1; i < _primitiveBufferOffsets[primitive].size(); ++i)
		{
			offsets.push_back(_primitiveBufferOffsets[primitive][i]);
		}

		return offsets;
	}

	uint32_t Mesh::primitiveCount() const
	{
		return _meshAsset->primitives.size();
	}

	MeshAsset* Mesh::meshAsset()
	{
		return _meshAsset;
	}

	void Mesh::updateData()
	{
		if(!_meshAsset->meshUpdated)
			return;
		for (size_t i = 0; i < _meshAsset->primitives.size(); ++i)
		{
			auto& p = _meshAsset->primitives[i];
			std::vector<byte> data = p.packedData();
			_stagingBuffer->setData(data, _primitiveBufferOffsets[i][0]);
		}

		SingleUseCommandBuffer cmdBuffer(device->transferPool());
		_dataBuffer->copy(_stagingBuffer, cmdBuffer.get(), size());
		cmdBuffer.submit(device->transferQueue());

		_meshAsset->meshUpdated = false;
	}
}