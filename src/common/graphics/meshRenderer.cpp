//
// Created by eli on 5/29/2022.
//

#include "meshRenderer.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "graphics.h"
#include "systems/transforms.h"
#include "ecs/entity.h"
#include "material.h"
#include "mesh.h"
#include "assets/types/materialAsset.h"

namespace graphics{


	MeshRenderer::MeshRenderer(SwapChain& swapChain, VulkanRuntime* vkr, EntityManager* em) : Renderer(swapChain), _vkr(*vkr), _em(*em)
	{

	}

	void MeshRenderer::render(VkCommandBuffer cmdBuffer)
	{
		if(_renderPass == VK_NULL_HANDLE)
			return;
		if(_extent.width == 0 || _extent.height == 0)
            return;
        startRenderPass(cmdBuffer);
 		glm::mat4 cameraMatrix = perspectiveMatrix() * transformMatrix();
		for(auto& mat :  _vkr.materials())
        {

			VkPipeline pipeline = getPipeline(mat.get());
			if(!pipeline)
				continue;
			vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			robin_hood::unordered_node_map<RenderObject, std::vector<glm::mat4>> meshTransforms;
			_em.systems().runUnmanagedSystem("fetchMRD", [&](SystemContext* ctx){
				ComponentFilter filter(ctx);
				filter.addComponent(Transform::def()->id, ComponentFilterFlags_Const);
				filter.addComponent(MeshRendererComponent::def()->id, ComponentFilterFlags_Const);
				_em.getEntities(filter).forEachNative([this, &mat, &meshTransforms, cameraMatrix](byte** components){
					MeshRendererComponent* mr = MeshRendererComponent::fromVirtual(components[1]);
                    if(!_vkr.meshes().hasIndex(mr->mesh))
                    {
                        Runtime::error("No mesh at index " + std::to_string(mr->mesh) + "!");
                        return;
                    }
					Mesh* mesh = _vkr.meshes()[mr->mesh].get();
					for (int j = 0; j < mesh->primitiveCount(); ++j)
					{
						if(mr->materials[j] != mat->asset()->runtimeID)
							continue;
						RenderObject ro{};
						ro.mesh = mesh;
						ro.primitive = j;
						meshTransforms[{mesh, (size_t)j}].push_back(cameraMatrix * Transform::fromVirtual(components[0])->value);
					}
				});
			});

			auto& transformBuffer = mat->transformBuffer(_swapChain.currentFrame());

			size_t transformCount = 0;
	        for(auto& renderObject : meshTransforms)
				transformCount += renderObject.second.size();

			if(transformCount * sizeof(glm::mat4) > transformBuffer.size())
			{
				size_t newSize = transformBuffer.size() * 2;
				while(transformCount * sizeof(glm::mat4) > newSize)
					newSize *= 2;
				transformBuffer.realocate(newSize);
			}

			size_t transformOffset = 0;
			for(auto& renderObject : meshTransforms)
			{
				Mesh* mesh = renderObject.first.mesh;
				size_t primitive = renderObject.first.primitive;
				VkBuffer b = mesh->buffer();
				std::vector<VkBuffer> vertexBuffers = {transformBuffer.get(), b, b};
				std::vector<VkDeviceSize> vertexBufferOffsets = {
						transformOffset,
						mesh->attributeBufferOffset(primitive, "POSITION"),
						mesh->attributeBufferOffset(primitive, "NORMAL")
				};

				mesh->updateData();
				transformBuffer.setData(renderObject.second, transformOffset);
				transformOffset += renderObject.second.size() * sizeof(glm::mat4);

				vkCmdBindVertexBuffers(cmdBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());
				vkCmdBindIndexBuffer(cmdBuffer, mesh->buffer(), mesh->indexBufferOffset(primitive), VK_INDEX_TYPE_UINT16);
				vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->pipelineLayout(), 0, 1, mat->descriptorSet(_swapChain.currentFrame()), 0, nullptr);
				vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(mesh->indexCount(primitive)), renderObject.second.size(), 0, 0, 0);
			}
		}
		endRenderPass(cmdBuffer);
	}

	void MeshRenderer::rebuild()
	{
		Renderer::rebuild();
		if(!_cachedPipelines.empty())
			for(auto p : _cachedPipelines)
				vkDestroyPipeline(graphics::device->get(), p.second, nullptr);
		_cachedPipelines.clear();
	}

	VkPipeline MeshRenderer::getPipeline(const Material* mat)
	{
		VkPipeline pipeline = VK_NULL_HANDLE;
		//TODO still want to refactor this
		try{
			pipeline = mat->pipeline(this);
		}catch(const std::exception& e)
		{
			//Runtime::warn("Tried to create pipeline from invalid material configuration: " + (std::string)e.what());
			return VK_NULL_HANDLE;
		}
		if(!_cachedPipelines.count(mat))
			_cachedPipelines.insert({mat, pipeline});
		return _cachedPipelines.at(mat);
	}

	MeshRenderer::~MeshRenderer()
	{
		if(!_cachedPipelines.empty())
			for(auto p : _cachedPipelines)
				vkDestroyPipeline(graphics::device->get(), p.second, nullptr);
		_cachedPipelines.clear();
	}

    glm::mat4 MeshRenderer::transformMatrix() const
    {
        return glm::inverse(glm::translate(glm::mat4(1), position) * glm::toMat4(rotation));
    }

    glm::mat4 MeshRenderer::perspectiveMatrix() const
    {
        glm::mat4 projection = glm::perspectiveLH(glm::radians(fov), static_cast<float>(_extent.width) / static_cast<float>(_extent.height), 0.1f, 100.0f);
        projection[1][1] *= -1;
        return projection;
    }
}