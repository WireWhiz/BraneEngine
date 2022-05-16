#include "client.h"
#include "runtime/runtime.h"

#include "networking/networking.h"
#include "graphics/graphics.h"
#include "assets/assembly.h"
#include "ecs/nativeTypes/transform.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "ecs/nativeTypes/assetComponents.h"
#include "ecs/nativeSystems/nativeSystems.h"
#include "assets/assetManager.h"

#include <utility/threadPool.h>
#include "asio/asio.hpp"
#include "networking/connection.h"

const char* Client::name()
{
	return "client";
}

Client::Client(Runtime& rt) : Module(rt)
{
	EntityManager& em = *(EntityManager*)rt.getModule("entityManager");
	AssetManager& am = *(AssetManager*)rt.getModule("assetManager");
	graphics::VulkanRuntime& vkr = *(graphics::VulkanRuntime*)rt.getModule("graphics");
	NetworkManager& nm = *(NetworkManager*)rt.getModule("networkManager");

	systems::addTransformSystem(em, rt.timeline());
	addAssetPreprocessors(am, vkr);

	ComponentSet headRootComponents;
	headRootComponents.add(AssemblyRoot::def());
	headRootComponents.add(TransformComponent::def());
	EntityID testHead = em.createEntity(headRootComponents);

	AssemblyRoot testHeadRoot{};
	testHeadRoot.id = AssetID("localhost/0000000000000F5F");

	em.setEntityComponent(testHead, testHeadRoot.toVirtual());

	TransformComponent tc{};
	tc.value = glm::scale(glm::mat4(1), {0.5, 0.5, 0.5});
	em.setEntityComponent(testHead, tc.toVirtual());

	graphics::Material* mat = new graphics::Material();
	mat->setVertex(vkr.loadShader(0));
	mat->setFragment(vkr.loadShader(1));
	//mat->addTextureDescriptor(vkr.loadTexture(0));
	mat->addBinding(0,sizeof(glm::vec3));
	mat->addBinding(1, sizeof(glm::vec3));
	mat->addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, 0);
	mat->addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, 0);
	vkr.initRenderer(vkr.addMaterial(mat));

	graphics::Material* mat2 = new graphics::Material();
	mat2->setVertex(vkr.loadShader(0));
	mat2->setFragment(vkr.loadShader(2));
	//mat->addTextureDescriptor(vkr.loadTexture(0));
	mat2->addBinding(0,sizeof(glm::vec3));
	mat2->addBinding(1, sizeof(glm::vec3));
	mat2->addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, 0);
	mat2->addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, 0);
}

void Client::addAssetPreprocessors(AssetManager& am, graphics::VulkanRuntime& vkr)
{
	am.addAssetPreprocessor(AssetType::assembly, [&am, &vkr](Asset* asset){
		auto assembly = (Assembly*)asset;
		for(auto& entity : assembly->entities)
		{
			for(auto& component : entity.components)
			{
				if(component.def() == MeshRendererComponent::def())
				{
					MeshRendererComponent* mr = MeshRendererComponent::fromVirtual(component.data());
					auto* meshAsset = am.getAsset<MeshAsset>(AssetID(assembly->meshes[mr->mesh]));
					if(meshAsset->pipelineID == -1)
						vkr.addMesh(meshAsset);
					mr->mesh = meshAsset->pipelineID;

					//TODO: process materials here as well
				}
			}
		}
	});
}
