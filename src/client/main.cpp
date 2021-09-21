// Client
#include <iostream>
#include "tests/tests.h"
#include "graphics/graphics.h"
#include "networking/client.h"

int main()
{
#ifdef DEBUG
	tests::runTests();
#endif
	std::cout << "BraneSurfer starting up\n";

	std::string serverAddress = Config::json()["network"]["asset server"].get("address", "127.0.0.1").asString();
	uint16_t serverPort = Config::json()["network"]["asset server"].get("tcp port", 80).asUInt();
	uint16_t serverSSLPort = Config::json()["network"]["asset server"].get("ssl port", 81).asUInt();
	net::ClientConnection cc;
	if (cc.connect(serverAddress, serverPort, net::ConnectionType::reliable))
		std::cout << "Connected to asset server!" << std::endl;

	std::this_thread::sleep_for(std::chrono::milliseconds(200)); //See if we're trying to send data before the handshake is done

	std::string text = "Hello there!";
	net::OMessage msg;
	msg.header.type = net::MessageType::one;
	msg.write(text.data(), text.size());
	cc.send(msg);
	while (cc.incoming().count() == 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	
	net::IMessage response = cc.incoming().pop_front().message;
	text.resize(response.header.size);
	response.read(text.data(), response.header.size);
	std::cout << "Recived: " << text << std::endl;
	

	std::cout << std::endl;

	graphics::VulkanRuntime vkr;
	EntityManager em;

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
	EntityID quadEntity  = em.createEntity({ Transform::def()->id(), graphics::MeshComponent::def()->id(), 2 });
	EntityID quadEntity2 = em.createEntity({ Transform::def()->id(), graphics::MeshComponent::def()->id(), 2 });

	while (!vkr.window()->closed())
	{
		vkr.updateWindow();
		vkr.updateUniformBuffer(em, 0);
		vkr.draw(em);
		
	}
	return 0;
}