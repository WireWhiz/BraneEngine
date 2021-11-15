#include "mesh.h"

namespace graphics
{
	Mesh::Mesh(std::vector<uint32_t> indices, std::vector<Vertex> vertices)
	{
		this->indices = indices;
		this->vertices = vertices;
		_locked = true;
		unlock();

		
		_stagingBuffer->setData(indices, 0);
		_stagingBuffer->setData(vertices, indices.size() * sizeof(uint32_t));

		_dataBuffer = new GraphicsBuffer(vertices.size() * sizeof(Vertex) + indices.size() * sizeof(uint32_t),
											 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
											 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		SingleUseCommandBuffer cmdBuffer(device->transferPool());
		_dataBuffer->copy(_stagingBuffer, cmdBuffer.get(), vertices.size() * sizeof(Vertex) + indices.size() * sizeof(uint32_t));
		cmdBuffer.submit(device->transferQueue());
	}
	Mesh::~Mesh()
	{
		if(!_locked)
				delete _stagingBuffer;
		delete _dataBuffer;

	}
	VkBuffer Mesh::data() const
	{
		return _dataBuffer->get();
	}
	size_t Mesh::vertexBufferOffset() const
	{
		return indices.size() * sizeof(uint32_t);
	}
	uint32_t Mesh::size()const
	{
		return _size;
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
			_stagingBuffer = new GraphicsBuffer(vertices.size() * sizeof(Vertex) + indices.size() * sizeof(uint32_t),
												VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
												VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			_locked = false;
		}
	}
}