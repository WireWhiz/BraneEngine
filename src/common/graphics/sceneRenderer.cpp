//
// Created by eli on 5/29/2022.
//

#include "sceneRenderer.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "graphics.h"
#include "systems/transforms.h"
#include "ecs/entity.h"
#include "material.h"
#include "mesh.h"
#include "assets/types/materialAsset.h"
#include "pointLightComponent.h"

namespace graphics{


	SceneRenderer::SceneRenderer(SwapChain& swapChain, VulkanRuntime* vkr, EntityManager* em) : Renderer(swapChain), _vkr(*vkr), _em(*em)
	{
		_renderDataBuffers.resize(swapChain.size());
		for(auto& b : _renderDataBuffers)
		{
			b.setFlags(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			b.realocate(sizeof(glm::mat4));
		}

		_pointLights.resize(swapChain.size());
		for(auto& b : _pointLights)
		{
			b.setFlags(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			b.realocate(sizeof(glm::mat4));
		}
	}

	SceneRenderer::~SceneRenderer()
	{
		if(!_cachedPipelines.empty())
			for(auto p : _cachedPipelines)
				vkDestroyPipeline(graphics::device->get(), p.second, nullptr);
		_cachedPipelines.clear();
	}

	void SceneRenderer::render(VkCommandBuffer cmdBuffer)
	{
		if(_renderPass == VK_NULL_HANDLE)
			return;
		if(_extent.width == 0 || _extent.height == 0)
            return;
		//Update lights
		std::vector<PointLightData> pointLights;
		_em.systems().runUnmanagedSystem("fetchMRD", [this, &pointLights](SystemContext* ctx)
		{
			ComponentFilter filter(ctx);
			filter.addComponent(Transform::def()->id, ComponentFilterFlags_Const);
			filter.addComponent(PointLightComponent::def()->id, ComponentFilterFlags_Const);
			_em.getEntities(filter).forEachNative([&pointLights](byte** components){
				Transform* transform = Transform::fromVirtual(components[0]);
				PointLightComponent* light = PointLightComponent::fromVirtual(components[1]);
				PointLightData data{transform->value[3], light->color};
				pointLights.push_back(data);
			});
		});
		updateLights(_swapChain.currentFrame(), pointLights);

		//Fetch mesh transforms, I beg of you to ignore this monstrosity of a data structure...
		robin_hood::unordered_node_map<uint32_t, robin_hood::unordered_node_map<RenderObject, std::vector<glm::mat4>>> meshTransforms;
		_em.systems().runUnmanagedSystem("fetchMRD", [&](SystemContext* ctx){
			ComponentFilter filter(ctx);
			filter.addComponent(Transform::def()->id, ComponentFilterFlags_Const);
			filter.addComponent(MeshRendererComponent::def()->id, ComponentFilterFlags_Const);
			_em.getEntities(filter).forEachNative([this, &meshTransforms](byte** components){
				MeshRendererComponent* mr = MeshRendererComponent::fromVirtual(components[1]);
				if(!_vkr.meshes().hasIndex(mr->mesh))
				{
					Runtime::error("No mesh at index " + std::to_string(mr->mesh) + "!");
					return;
				}
				Mesh* mesh = _vkr.meshes()[mr->mesh].get();
				if(!mesh)
					return;
				for (int j = 0; j < mesh->primitiveCount(); ++j)
				{
					if(j == mr->materials.size())
						break;
					RenderObject ro{};
					ro.mesh = mesh;
					ro.primitive = j;
					meshTransforms[mr->materials[j]][{mesh, (size_t)j}].push_back(Transform::fromVirtual(components[0])->value);
				}
			});
		});

		//Render meshes
        startRenderPass(cmdBuffer);
 		glm::mat4 cameraMatrix = perspectiveMatrix() * transformMatrix();
	    _renderDataBuffers[_swapChain.currentFrame()].setData(cameraMatrix, 0);
		for(auto& mat :  _vkr.materials())
        {
			VkPipeline pipeline = getPipeline(mat.get());
			if(!pipeline)
				continue;
			vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			auto& transformBuffer = mat->transformBuffer(_swapChain.currentFrame());

			size_t transformCount = 0;
	        for(auto& renderObject : meshTransforms[mat->asset()->runtimeID])
				transformCount += renderObject.second.size();

			if(transformCount * sizeof(glm::mat4) > transformBuffer.size())
			{
				size_t newSize = transformBuffer.size() * 2;
				while(transformCount * sizeof(glm::mat4) > newSize)
					newSize *= 2;
				mat->reallocateTransformBuffer(_swapChain.currentFrame(), newSize);
			}

	        mat->bindTransformUniformBuffer(_swapChain.currentFrame(), _renderDataBuffers[_swapChain.currentFrame()]);
			mat->bindPointLightBuffer(_swapChain.currentFrame(), _pointLights[_swapChain.currentFrame()]);

	        vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->pipelineLayout(), 0, 1, mat->descriptorSet(_swapChain.currentFrame()), 0, nullptr);

	        uint32_t transformOffset = 0;
			for(auto& renderObject : meshTransforms[mat->asset()->runtimeID])
			{
				Mesh* mesh = renderObject.first.mesh;
				size_t primitive = renderObject.first.primitive;
				VkBuffer b = mesh->buffer();
				std::vector<VkBuffer> vertexBuffers = {b, b};
				std::vector<VkDeviceSize> vertexBufferOffsets = {
						mesh->attributeBufferOffset(primitive, "POSITION"),
						mesh->attributeBufferOffset(primitive, "NORMAL")
				};


				mesh->updateData();
				transformBuffer.setData(renderObject.second, transformOffset);
				uint32_t instanceOffset = transformOffset / sizeof(glm::mat4);
				vkCmdPushConstants(cmdBuffer, mat->pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &instanceOffset);
				transformOffset += renderObject.second.size() * sizeof(glm::mat4);

				vkCmdBindVertexBuffers(cmdBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());
				vkCmdBindIndexBuffer(cmdBuffer, mesh->buffer(), mesh->indexBufferOffset(primitive), VK_INDEX_TYPE_UINT16);
				vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(mesh->indexCount(primitive)), renderObject.second.size(), 0, 0, 0);
			}
		}
		endRenderPass(cmdBuffer);
	}

	void SceneRenderer::rebuild()
	{
		Renderer::rebuild();
		if(!_cachedPipelines.empty())
			for(auto p : _cachedPipelines)
				vkDestroyPipeline(graphics::device->get(), p.second, nullptr);
		_cachedPipelines.clear();
	}

	VkPipeline SceneRenderer::getPipeline(const Material* mat)
	{
		if(_cachedPipelines.count(mat))
			return _cachedPipelines.at(mat);
		//TODO still want to refactor this
		try{
			VkPipeline pipeline = mat->pipeline(this);
			_cachedPipelines.insert({mat, pipeline});
			return pipeline;
		}catch(const std::exception& e)
		{
			//Runtime::warn("Tried to create pipeline from invalid material configuration: " + (std::string)e.what());
		}
		return VK_NULL_HANDLE;
	}

    glm::mat4 SceneRenderer::transformMatrix() const
    {
        return glm::inverse(glm::translate(glm::mat4(1), position) * glm::toMat4(rotation));
    }

    glm::mat4 SceneRenderer::perspectiveMatrix() const
    {
        glm::mat4 projection = glm::perspectiveLH(glm::radians(fov), static_cast<float>(_extent.width) / static_cast<float>(_extent.height), 0.1f, 100.0f);
        projection[1][1] *= -1;
        return projection;
    }

	void SceneRenderer::updateLights(size_t frame, std::vector<PointLightData>& lights)
	{
		if(lights.empty())
			return;
		auto& buffer = _pointLights[frame];
		//16 because vulkan is dumb and doesn't let you tightly pack ints
		if(buffer.size() < 16 + lights.size() * sizeof(PointLightData))
		{
			size_t newSize = buffer.size() * 2;
			while(sizeof(uint32_t) + lights.size() * sizeof(PointLightData) > newSize)
				newSize *= 2;
			buffer.realocate(newSize);
		}
		buffer.setData(static_cast<uint32_t>(lights.size()), 0);
		buffer.setData(lights, 16);
	}

	void SceneRenderer::reloadMaterial(Material* material)
	{
		auto pipeline = _cachedPipelines.find(material);
		if(pipeline != _cachedPipelines.end())
		{
			vkDestroyPipeline(graphics::device->get(), pipeline->second, nullptr);
			_cachedPipelines.erase(pipeline);
		}
	}
}