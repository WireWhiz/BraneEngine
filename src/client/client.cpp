#include "client.h"
#include "asio/asio.hpp"
#include "networking/connection.h"
#include "assets/assetManager.h"
#include "graphics/graphics.h"
#include "graphics/graphicAssetPreprocessors.h"
#include "assets/assembly.h"
#include "ecs/nativeTypes/transform.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "ecs/nativeTypes/assetComponents.h"
#include "ecs/nativeSystems/nativeSystems.h"

void Client::run()
{
	std::cout << "BraneSurfer starting up\n";
	ThreadPool::init(4);
	FileManager fm;
	NetworkManager nm;


	AssetManager am(fm, nm);
	EntityManager em;
	graphics::VulkanRuntime vkr;

	systems::addTransformSystem(em);

	GraphicAssetPreprocessors::addAssetPreprocessors(am, vkr);
	nm.startAssetAcceptorSystem(em, am);
	am.startAssetLoaderSystem(em);


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
	//vkr.initRenderer(vkr.addMaterial(mat2));

	while (!vkr.window()->closed())
	{
		vkr.updateWindow();
		em.runSystems();
		vkr.draw(em); // Replace with system as well
	}

	nm.stop();
	ThreadPool::cleanup();
}