#include "client.h"
#include "runtime/runtime.h"

#include "networking/networking.h"
#include "graphics/graphics.h"
#include "assets/assembly.h"
#include "systems/transforms.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "ecs/nativeTypes/assetComponents.h"
#include "assets/assetManager.h"

#include <utility/threadPool.h>
#include <asio.hpp>
#include "networking/connection.h"

const char* Client::name()
{
	return "client";
}

Client::Client() :
_am(*Runtime::getModule<AssetManager>()),
_nm(*Runtime::getModule<NetworkManager>())
{
	_am.setFetchCallback([this](auto id, auto incremental){return fetchAssetCallback(id, incremental);});

	EntityManager& em = *Runtime::getModule<EntityManager>();
	graphics::VulkanRuntime& vkr = *Runtime::getModule<graphics::VulkanRuntime>();

	addAssetPreprocessors(_am, vkr);

	ComponentSet headRootComponents;
	headRootComponents.add(AssemblyRoot::def()->id);
	headRootComponents.add(Transform::def()->id);
	EntityID testHead = em.createEntity(headRootComponents);

	AssemblyRoot testHeadRoot{};
	testHeadRoot.id = AssetID("localhost/0000000000000F5F");

	em.setEntityComponent(testHead, testHeadRoot.toVirtual());

	Transform tc{};
	tc.value = glm::scale(glm::mat4(1), {0.5, 0.5, 0.5});
	em.setEntityComponent(testHead, tc.toVirtual());

	auto* mat = new graphics::Material();
	mat->setVertex(vkr.loadShader(0));
	mat->setFragment(vkr.loadShader(1));
	//mat->addTextureDescriptor(vkr.loadTexture(0));
	mat->addBinding(0,sizeof(glm::vec3));
	mat->addBinding(1, sizeof(glm::vec3));
	mat->addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, 0);
	mat->addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, 0);
	vkr.addMaterial(mat);

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
				if(component.description() == MeshRendererComponent::def())
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

AsyncData<Asset*> Client::fetchAssetCallback(const AssetID& id, bool incremental)
{
	AsyncData<Asset*> asset;
	_nm.async_connectToAssetServer(id.serverAddress, Config::json()["network"]["tcp_port"].asUInt(), [this, id, incremental, asset](bool connected){
		if(!connected)
		{
			asset.setError("Could not connect to server: " + id.serverAddress);
			std::cerr << "Could not get asset: " << id << std::endl;
			return;
		}
		if (incremental)
		{
			_nm.async_requestAssetIncremental(id).then([this, asset, id](Asset* data){
				asset.setData(data);

			});
		}
		else
		{
			AsyncData<Asset*> assetToSave;
			_nm.async_requestAsset(id).then([this, asset, id](Asset* data){
				asset.setData(data);
			});
		}
	});;
	return asset;
}
