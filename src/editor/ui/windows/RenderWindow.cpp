//
// Created by eli on 5/21/2022.
//

#include "RenderWindow.h"
#include "../editorUI.h"
#include "graphics/graphics.h"
#include "graphics/MeshRenderer.h"
#include "networking/networking.h"
#include "assets/assetManager.h"
#include "ecs/nativeSystems/nativeSystems.h"
#include "ecs/core/component.h"
#include "ecs/nativeTypes/assetComponents.h"

RenderWindow::RenderWindow(EditorUI& ui) : EditorWindow(ui)
{
	auto& am = (*(AssetManager*)ui.runtime().getModule("assetManager"));
	auto& nm = (*(NetworkManager*)ui.runtime().getModule("networkManager"));
	am.setFetchCallback([&](auto id, auto incremental){
		AsyncData<Asset*> asset;
		nm.async_connectToAssetServer(id.serverAddress, Config::json()["network"]["tcp_port"].asUInt(), [&am, &nm, id, incremental, asset](bool connected){
			if(!connected)
			{
				asset.setError("Could not connect to server: " + id.serverAddress);
				std::cerr << "Could not get asset: " << id << std::endl;
				return;
			}
			if (incremental)
			{
				nm.async_requestAssetIncremental(id, am).then([asset, id](Asset* data){
					asset.setData(data);
				});
			}
			else
			{
				AsyncData<Asset*> assetToSave;
				nm.async_requestAsset(id, am).then([asset, id](Asset* data){
					asset.setData(data);
				});
			}
		});;
		return asset;
	});

	EntityManager& em = *(EntityManager*)ui.runtime().getModule("entityManager");
	graphics::VulkanRuntime* vkr = ((graphics::VulkanRuntime*)ui.runtime().getModule("graphics"));
	_renderer = vkr->createRenderer<graphics::MeshRenderer>(vkr, &em);
	_renderer->setClearColor({.2,.2,.2,1});
	_swapChain = vkr->swapChain();
	_renderer->position = {1,2,-6};
	_renderer->rotation = glm::quatLookAt(-_renderer->position, {0, 1, 0});

	systems::addTransformSystem(em, ui.runtime().timeline());

	auto* mat = new graphics::Material();
	mat->setVertex(vkr->loadShader(0));
	mat->setFragment(vkr->loadShader(1));
	//mat->addTextureDescriptor(vkr.loadTexture(0));
	mat->addBinding(0,sizeof(glm::vec3));
	mat->addBinding(1, sizeof(glm::vec3));
	mat->addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, 0);
	mat->addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, 0);
	vkr->addMaterial(mat);

	am.addAssetPreprocessor(AssetType::assembly, [&am, mat, vkr](Asset* asset){
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
						vkr->addMesh(meshAsset);
					mr->mesh = meshAsset->pipelineID;
					entity.components.emplace_back(mat->component());

					//TODO: process materials here as well
				}
			}
		}
	});

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


}

RenderWindow::~RenderWindow()
{
	if(_texture)
		delete _texture;
}

void RenderWindow::update()
{
	_frameCount++;

	if(_queueReload)
	{
		vkDeviceWaitIdle(graphics::device->get());
		_renderer->clearTarget();
		if(!_imGuiBindings.empty())
			vkFreeDescriptorSets(graphics::device->get(), _ui.descriptorPool(), _imGuiBindings.size(), _imGuiBindings.data());
		_imGuiBindings.resize(0);
		delete _texture;
		_texture = nullptr;
		_queueReload = false;
	}

	if(!_texture && (_windowSize.width != 0 && _windowSize.height != 0))
	{
		_texture = new graphics::RenderTexture(_windowSize, true, *_swapChain);
		auto& images = _texture->images();
		_imGuiBindings.resize(0);
		for(auto image : images)
		{
			_imGuiBindings.push_back(ImGui_ImplVulkan_AddTexture(_texture->sampler(), image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		}
		_renderer->setTarget(_texture);
	}
}

void RenderWindow::draw()
{
	if(ImGui::Begin("Render", nullptr, ImGuiWindowFlags_None)){
		auto window = ImGui::GetContentRegionAvail();
		_windowSize = {static_cast<uint32_t>(glm::floor(glm::max((float)0,window.x))), static_cast<uint32_t>(glm::floor(glm::max((float)0,window.y)))};
		if(_windowSize.width != 0 && _windowSize.height != 0)
		{
			if(_texture && (_windowSize.width != _texture->size().width ||
			                _windowSize.height != _texture->size().height))
			{
				_queueReload = true;
			}
			if(_texture)
				ImGui::Image(_imGuiBindings[_swapChain->currentFrame()], window);
			ImGui::DragFloat3("camera position", &position.x);
			ImGui::DragFloat2("camera rotation", &rotation.x);
			ImGui::DragFloat("camera zoom", &zoom);
			_renderer->position = position;
			_renderer->rotation = glm::angleAxis(glm::radians(rotation.y), glm::vec3(0.0f,1.0f,0.0f)) * glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f,0.0f,0.0f));
		}
	}
	ImGui::End();
}




