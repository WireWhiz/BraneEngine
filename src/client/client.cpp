#include "client.h"
#include "asio/asio.hpp"
#include "networking/connection.h"

void Client::run()
{
	std::cout << "BraneSurfer starting up\n";

	std::string serverAddress = Config::json()["network"]["runtime server"].get("address", "127.0.0.1").asString();
	uint16_t serverPort = Config::json()["network"]["runtime server"].get("tcp port", 2001).asUInt();
	uint16_t serverSSLPort = Config::json()["network"]["runtime server"].get("ssl port", 2002).asUInt();

	EntityManager em;
	//graphics::VulkanRuntime vkr;


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

	asio::io_context ctx;
	asio::ip::tcp::resolver tcpresolver(ctx);
	auto tcpEndpoints = tcpresolver.resolve(serverAddress, std::to_string(serverPort));

	std::cout << "Attempting to connect to: " << serverAddress << "::" << serverPort << std::endl;
	net::TCPConnection connection(net::Connection::Owner::client, ctx, net::tcp_socket(ctx), &em);
	connection.connectToServer(tcpEndpoints);

	ThreadPool::enqueue([&]() {
		ctx.run();
	});

	NativeForEach nfe(std::vector<const ComponentAsset*>{net::IMessageComponent::def()}, & em);

	while (true)//!vkr.window()->closed())
	{
		//vkr.updateWindow();
		em.runSystems();
		em.forEach(nfe.id(), [&](byte** components) {
			net::IMessageComponent* m = net::IMessageComponent::fromVirtual(components[0]);
			std::cout << m->message << std::endl;
		});
		//vkr.updateUniformBuffer(em, 0);//Replace that with a systems
		//vkr.draw(em); // Replace with system as well

	}
}