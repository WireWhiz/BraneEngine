//
// Created by eli on 5/29/2022.
//

#include "MeshRenderer.h"
#include "graphics.h"

namespace graphics{


	MeshRenderer::MeshRenderer(SwapChain& swapChain, VulkanRuntime* vkr, EntityManager* em) : Renderer(swapChain), _vkr(*vkr), _em(*em)
	{

	}

	void MeshRenderer::render(VkCommandBuffer cmdBuffer)
	{
		if(_renderPass == VK_NULL_HANDLE)
			return;
		startRenderPass(cmdBuffer);
		if(_extent.width == 0 || _extent.height == 0)
		{
			endRenderPass(cmdBuffer);
			return;
		}
		glm::mat4 transform = glm::inverse(glm::translate(glm::mat4(1), position) * glm::toMat4(rotation));
		glm::mat4 projection = glm::perspective(glm::radians(fov), static_cast<float>(_extent.width) / static_cast<float>(_extent.height), 0.1f, 100.0f);
		projection[1][1] *= -1;
		glm::mat4 cameraMatrix = projection * transform;
		_vkr.materials().forEach([this, cmdBuffer, cameraMatrix, transform](auto& mat){
			vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, getPipeline(mat.get()));

			const NativeForEach& nfe = getForEach(mat.get());
			std::vector<RenderObject> meshes;
			_em.forEach(nfe.id(), [this, &meshes, &nfe](byte** components){
				MeshRendererComponent* mr = MeshRendererComponent::fromVirtual(components[nfe.getComponentIndex(1)]);
				Mesh* mesh = _vkr.meshes()[mr->mesh].get();
				for (int j = 0; j < mesh->primitiveCount(); ++j)
				{
					RenderObject ro{};
					ro.mesh = mesh;
					ro.primitive = j;
					ro.transform = TransformComponent::fromVirtual(components[nfe.getComponentIndex(0)])->value;
					//_renderCache[/*mr->materials[j]*/0].push_back(ro);
					//TODO actually add in support for more than one material
					meshes.push_back(ro);
				}
			});
			for(auto& mesh : meshes)
			{
				VkBuffer b = mesh.mesh->buffer();
				std::vector<VkBuffer> vertexBuffers;
				vertexBuffers.resize(2, b);
				std::vector<VkDeviceSize> vertexBufferOffsets = {
						mesh.mesh->attributeBufferOffset(mesh.primitive, "POSITION"),
						mesh.mesh->attributeBufferOffset(mesh.primitive, "NORMAL")
				};
				for(auto offset : vertexBufferOffsets)
				{
					assert(offset < mesh.mesh->size());
				}
				mesh.mesh->updateData();
				vkCmdBindVertexBuffers(cmdBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

				vkCmdBindIndexBuffer(cmdBuffer, mesh.mesh->buffer(), mesh.mesh->indexBufferOffset(mesh.primitive), VK_INDEX_TYPE_UINT16);

				vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->pipelineLayout(), 0, 1, mat->descriptorSet(_swapChain.currentFrame()), 0, nullptr);

				MeshPushConstants constants{};
				constants.render_matrix = cameraMatrix * mesh.transform;
				constants.objectToWorld = mesh.transform;

				static auto start = std::chrono::high_resolution_clock::now();
				auto currentTime = std::chrono::high_resolution_clock::now();
				float delta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - start).count();

				glm::mat4 lightPos = glm::translate(glm::rotate(glm::mat4(1), glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3{2, 2, -4});;
				constants.lightPosition = lightPos * glm::vec4{1,1,1,1};

				vkCmdPushConstants(cmdBuffer, mat->pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MeshPushConstants), &constants);


				vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(mesh.mesh->indexCount(mesh.primitive)), 1, 0, 0, 0);
			}

		});
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
		if(!_cachedPipelines.count(mat))
			_cachedPipelines.insert({mat, mat->pipeline(this)});
		return _cachedPipelines.at(mat);
	}

	NativeForEach& MeshRenderer::getForEach(const Material* mat)
	{
		if(!_cachedForEach.count(mat)){
			std::vector<const ComponentAsset*> components = {TransformComponent::def(), MeshRendererComponent::def(), mat->component()};
			_cachedForEach.insert({mat, NativeForEach(components, &_em)});
		}
		return _cachedForEach.at(mat);
	}
}