#include "client.h"
#include "asio/asio.hpp"
#include "networking/connection.h"
#include "assets/assetManager.h"
#include "graphics/graphics.h"
#include "assets/assembly.h"
#include "ecs/nativeTypes/transform.h"
#include "ecs/nativeTypes/meshRenderer.h"

void Client::run()
{
	std::cout << "BraneSurfer starting up\n";

	FileManager fm;
	NetworkManager nm;
	nm.start();

	AssetManager am(fm, nm);
	EntityManager em;
	graphics::VulkanRuntime vkr;


	// Add networking components and systems to entity manager
	// Run the systems in the run loop


	/*MeshAsset quadMeshAsset;
	quadMeshAsset.name = "quadMeshAsset";
	quadMeshAsset.indices = { 0, 1, 2, 2, 3, 0 };
	quadMeshAsset.positions = {{-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}, {-0.5f, 0.5f, 0.0f}};
	quadMeshAsset.uvs.push_back({{1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}});

	std::unique_ptr<graphics::Mesh> quad = std::make_unique<graphics::Mesh>(&quadMeshAsset);
	uint32_t quadHandle = vkr.addMesh(std::move(quad));*/

	AssetID testHeadAssembly("localhost/00000000000009A5");
	Assembly* testAsset = am.getAsset<Assembly>(testHeadAssembly);
	vkr.addMesh(std::make_unique<graphics::Mesh>(am.getAsset<MeshAsset>(*(testAsset->meshes.end() - 3))));
	//vkr.addMesh(std::make_unique<graphics::Mesh>(am.getAsset<MeshAsset>(*(testAsset->meshes.end() - 8))));

	ComponentSet quadComponents;
	quadComponents.add(comps::TransformComponent::def());
	quadComponents.add(comps::MeshRendererComponent::def());
	EntityID quadEntity = em.createEntity(quadComponents);
	//EntityID quadEntity2 = em.createEntity(quadComponents);

	comps::TransformComponent transform{};
	comps::MeshRendererComponent meshRenderer{};
	transform.value = glm::mat4(1);
	meshRenderer.mesh = 0;
	meshRenderer.materials = {0,1};

	em.setEntityComponent(quadEntity, transform.toVirtual());
	em.setEntityComponent(quadEntity, meshRenderer.toVirtual());

	//meshRenderer.mesh = 1;

	//em.setEntityComponent(quadEntity2, transform.toVirtual());
	//em.setEntityComponent(quadEntity2, meshRenderer.toVirtual());


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
	vkr.initRenderer(vkr.addMaterial(mat2));

	while (!vkr.window()->closed())
	{
		vkr.updateWindow();
		em.runSystems();
		vkr.updateUniformBuffer(em);//Replace that with a systems
		vkr.draw(em); // Replace with system as well

	}
}