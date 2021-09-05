#include "renderer.h"

namespace graphics
{
	Renderer::Renderer(EntityManager& em, Material* material)
	{
        _material = material;
        _forEachID = em.getForEachID({ Transform::def()->id(), MeshComponent::def()->id(), material->componentID() });

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
    void Renderer::createRenderBuffers(EntityManager& em, SwapChain* swapChain, std::unordered_map<MeshID,std::unique_ptr<Mesh>>& meshes, glm::mat4x4 cameraMatrix, VkCommandBufferInheritanceInfo& inheritanceInfo, size_t frame)
    {
        size_t count = em.forEachCount(_forEachID);
        if(count == 0)
            return;
        std::vector<glm::mat4x4> transforms(count);
        std::vector<MeshID> meshIndicies(count);

        size_t index = 0;
        em.forEach(_forEachID, [&index, &transforms, &meshIndicies](byte** components){
            meshIndicies[index] = MeshComponent::fromVirtual(components[1])->id;
            transforms[index] = Transform::fromVirtual(components[0])->value;
            index++;
        });

        

        if (_drawBuffers[frame].size() < count)
        {
            size_t start = _drawBuffers[frame].size();
            _drawBuffers[frame].resize(count);
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = device->graphicsPool();
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            allocInfo.commandBufferCount = (uint32_t)(count - start);

            if (vkAllocateCommandBuffers(device->get(), &allocInfo, &_drawBuffers[frame][start]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate command buffers!");
            }
        }
        

        for (size_t i = 0; i < _drawBuffers[frame].size(); i++)
        {
            vkResetCommandBuffer(_drawBuffers[frame][i], 0); // No flags so that the buffer will hold onto memory

            Mesh* mesh = meshes[meshIndicies[i]].get();

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            beginInfo.pInheritanceInfo = &inheritanceInfo;

            if (vkBeginCommandBuffer(_drawBuffers[frame][i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            vkCmdBindPipeline(_drawBuffers[frame][i], VK_PIPELINE_BIND_POINT_GRAPHICS, _material->pipeline());

            vkCmdBindIndexBuffer(_drawBuffers[frame][i], mesh->data(), 0, VK_INDEX_TYPE_UINT32);

            VkBuffer vertexBuffers[] = { mesh->data() };
            VkDeviceSize offsets[] = { static_cast<uint32_t>(mesh->indices.size() * sizeof(uint32_t)) };
            vkCmdBindVertexBuffers(_drawBuffers[frame][i], 0, 1, vertexBuffers, offsets);

            vkCmdBindDescriptorSets(_drawBuffers[frame][i], VK_PIPELINE_BIND_POINT_GRAPHICS, _material->pipelineLayout(), 0, 1, &_descriptorSets[frame], 0, nullptr);

            MeshPushConstants constants{};
            constants.render_matrix = cameraMatrix * transforms[i];

            vkCmdPushConstants(_drawBuffers[frame][i], _material->pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

            vkCmdDrawIndexed(_drawBuffers[frame][i], static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);

            if (vkEndCommandBuffer(_drawBuffers[frame][i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }


        }

    }
    std::vector<VkCommandBuffer>* Renderer::getBuffers(size_t frame)
    {
        return &_drawBuffers[frame];
    }
    void RendererData::getVariableIndicies(std::vector<NativeVarDef>& variables)
    {
        variables.push_back(NativeVarDef(offsetof(RendererData, disabled), virtualBool));
    }
}