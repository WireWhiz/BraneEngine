#include "renderer.h"

namespace graphics
{
	Renderer::Renderer(Material* material)
	{
        _material = material;
        _drawBuffers.resize(SwapChain::size());
        createDescriptorSets(SwapChain::size());
	}
	void Renderer::createDescriptorSets(size_t count)
	{
        //Create sets
        if (_descriptorSets.size() < count)
        {
            size_t currentSize = _descriptorSets.size();
            std::vector<VkDescriptorSetLayout> layouts(count - currentSize, _material->descriptorLayout());
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = _material->descriptorPool();
            allocInfo.descriptorSetCount = static_cast<uint32_t>(count - currentSize);
            allocInfo.pSetLayouts = layouts.data();

            _descriptorSets.resize(count);
            if (vkAllocateDescriptorSets(device->get(), &allocInfo, &_descriptorSets[currentSize]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }
        }
        

        for (size_t i = 0; i < count; i++)
        {
            std::vector<VkWriteDescriptorSet> descriptorWrites;

            /*
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i].get();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = _descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            */
            std::vector<VkDescriptorImageInfo> descriptorImages; // If we don't have this vector, referenced structs go out of scope
            _material->getImageDescriptors(descriptorWrites,  descriptorImages, _descriptorSets[i]);

            vkUpdateDescriptorSets(device->get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
	}
	std::vector<VkCommandBuffer> Renderer::createRenderBuffers(SwapChain* swapChain, std::vector<RenderObject>& meshes, glm::mat4x4 cameraMatrix, VkCommandBufferInheritanceInfo& inheritanceInfo, size_t frame)
    {
        if (_drawBuffers[frame].size() < meshes.size())
        {
            size_t start = _drawBuffers[frame].size();
            _drawBuffers[frame].resize(meshes.size());
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = device->graphicsPool();
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            allocInfo.commandBufferCount = (uint32_t)(meshes.size() - start);

            if (vkAllocateCommandBuffers(device->get(), &allocInfo, &_drawBuffers[frame][start]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate command buffers!");
            }
        }

	    std::vector<VkCommandBuffer> retBuffers;
		retBuffers.reserve(meshes.size());
        for (size_t i = 0; i < meshes.size(); i++)
        {

            vkResetCommandBuffer(_drawBuffers[frame][i], 0); // No flags so that the buffer will hold onto memory

            RenderObject& mesh = meshes[i];

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            beginInfo.pInheritanceInfo = &inheritanceInfo;

            if (vkBeginCommandBuffer(_drawBuffers[frame][i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            vkCmdBindPipeline(_drawBuffers[frame][i], VK_PIPELINE_BIND_POINT_GRAPHICS, _material->pipeline());

            auto vertexBuffers = mesh.mesh->vertexBuffers(mesh.primitive);
			auto vertexBufferOffsets = mesh.mesh->vertexBufferOffsets(mesh.primitive);
			for(auto offset : vertexBufferOffsets)
			{
				assert(offset < mesh.mesh->size());
			}
            vkCmdBindVertexBuffers(_drawBuffers[frame][i], 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

	        vkCmdBindIndexBuffer(_drawBuffers[frame][i], mesh.mesh->indexBuffer(mesh.primitive), mesh.mesh->indexBufferOffset(mesh.primitive), VK_INDEX_TYPE_UINT16);

            vkCmdBindDescriptorSets(_drawBuffers[frame][i], VK_PIPELINE_BIND_POINT_GRAPHICS, _material->pipelineLayout(), 0, 1, &_descriptorSets[frame], 0, nullptr);

            MeshPushConstants constants{};
            constants.render_matrix = cameraMatrix * mesh.transform;
			constants.objectToWorld = mesh.transform;

	        static auto start = std::chrono::high_resolution_clock::now();
	        auto currentTime = std::chrono::high_resolution_clock::now();
	        float delta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - start).count();

			glm::mat4 lightPos = glm::translate(glm::rotate(glm::mat4(1), delta * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3{2, 2, -4});;
			constants.lightPosition = lightPos * glm::vec4{1,1,1,1};

            vkCmdPushConstants(_drawBuffers[frame][i], _material->pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MeshPushConstants), &constants);


            vkCmdDrawIndexed(_drawBuffers[frame][i], static_cast<uint32_t>(mesh.mesh->vertexCount(mesh.primitive)), 1, 0, 0, 0);

            if (vkEndCommandBuffer(_drawBuffers[frame][i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }
			retBuffers.push_back(_drawBuffers[frame][i]);

        }
		return retBuffers;

    }
}