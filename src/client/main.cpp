#include <iostream>
#include "tests/tests.h"
#include "graphics/graphics.h"

int main()
{
#ifdef DEBUG
	tests::runTests();
#endif
	std::cout << "BraneSurfer starting up\n";

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
	EntityID quadEntity = em.createEntity({ Transform::def()->id(), graphics::MeshComponent::def()->id(), 2 });
	EntityID quadEntity2 = em.createEntity({ Transform::def()->id(), graphics::MeshComponent::def()->id(), 2 });

	while (!vkr.window()->closed())
	{
		vkr.updateWindow();
		vkr.updateUniformBuffer(em, 0);
		vkr.draw(em);

	}
	return 0;
}