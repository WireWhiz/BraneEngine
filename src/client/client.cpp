#include "client.h"

void Client::run()
{
	std::cout << "BraneSurfer starting up\n";

	std::string serverAddress = Config::json()["network"]["runtime server"].get("address", "127.0.0.1").asString();
	uint16_t serverPort = Config::json()["network"]["runtime server"].get("tcp port", 80).asUInt();
	uint16_t serverSSLPort = Config::json()["network"]["runtime server"].get("ssl port", 81).asUInt();

	EntityManager em;
	graphics::VulkanRuntime vkr;


	// Add networking components and systems to entity manager
	// Run the systems in the run loop

	/*
	std::unique_ptr<graphics::Mesh> quad = std::make_unique<graphics::Mesh>(std::vector<uint32_t>({ 0, 1, 2, 2, 3, 0 }),
																			std::vector<graphics::Vertex>({
																			   {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
																			   {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
																			   {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
																			   {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
																										  }));
	vkr.addMesh(std::move(quad), 0);

	graphics::Material* mat = vkr.createMaterial(0);
	mat->setVertex(vkr.loadShader(0));
	mat->setFragment(vkr.loadShader(1));
	mat->addTextureDescriptor(vkr.loadTexture(0));
	em.regesterComponent(mat->createUniformComponent(2, {}, 16));
	vkr.initMaterial(em, 0);

	em.regesterComponent(*Transform::def());
	em.regesterComponent(*graphics::MeshComponent::def()); //Regester mesh component
	EntityID quadEntity = em.createEntity({ Transform::def()->id(), graphics::MeshComponent::def()->id(), 2 });
	EntityID quadEntity2 = em.createEntity({ Transform::def()->id(), graphics::MeshComponent::def()->id(), 2 });
	*/


	while (!vkr.window()->closed())
	{
		vkr.updateWindow();
		vkr.updateUniformBuffer(em, 0);//Replace that with a systems
		vkr.draw(em); // Replace with system as well

	}
}