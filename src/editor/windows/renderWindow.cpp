//
// Created by eli on 5/21/2022.
//

#include "renderWindow.h"
#include <ui/gui.h>
#include "graphics/graphics.h"
#include "graphics/meshRenderer.h"
#include "networking/networking.h"
#include "assets/assetManager.h"
#include "ecs/core/component.h"
#include "ecs/nativeTypes/assetComponents.h"

SetEntityFocusEvent::SetEntityFocusEvent(const std::string& name, EntityID focus) : GUIEvent(name), _focus{focus}
{

}

EntityID SetEntityFocusEvent::focus() const
{
	return _focus;
}

RenderWindow::RenderWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{
	auto& am = *Runtime::getModule<AssetManager>();
	auto& nm = *Runtime::getModule<NetworkManager>();
	am.setFetchCallback([&](auto id, auto incremental){
		AsyncData<Asset*> asset;
		nm.async_connectToAssetServer(id.serverAddress, Config::json()["network"]["tcp_port"].asUInt(), [&nm, id, incremental, asset](bool connected){
			if(!connected)
			{
				asset.setError("Could not connect to server: " + id.serverAddress);
				std::cerr << "Could not get asset: " << id << std::endl;
				return;
			}
			if (incremental)
			{
				nm.async_requestAssetIncremental(id).then([asset, id](Asset* data){
					asset.setData(data);
				});
			}
			else
			{
				AsyncData<Asset*> assetToSave;
				nm.async_requestAsset(id).then([asset, id](Asset* data){
					asset.setData(data);
				});
			}
		});
		return asset;
	});

	EntityManager& em = *Runtime::getModule<EntityManager>();
	graphics::VulkanRuntime* vkr = Runtime::getModule<graphics::VulkanRuntime>();
	_renderer = vkr->createRenderer<graphics::MeshRenderer>(vkr, &em);
	_renderer->setClearColor({.2,.2,.2,1});
	_swapChain = vkr->swapChain();

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
				if(component.description() == MeshRendererComponent::def())
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

	/*ComponentSet headRootComponents;
	headRootComponents.add(AssemblyRoot::def()->id);
	headRootComponents.add(TransformComponent::def()->id);
	EntityID testHead = em.createEntity(headRootComponents);

	AssemblyRoot testHeadRoot{};
	testHeadRoot.id = AssetID("localhost/0000000000000F5F");

	em.setEntityComponent(testHead, testHeadRoot.toVirtual());

	TransformComponent tc{};
	tc.value = glm::scale(glm::mat4(1), {0.5, 0.5, 0.5});
	em.setEntityComponent(testHead, tc.toVirtual());*/
}

RenderWindow::~RenderWindow()
{
	vkDeviceWaitIdle(graphics::device->get());
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

			if(ImGui::IsWindowFocused())
			{
				//TODO add in sensitivity settings for all these
				zoom = glm::min<float>(0,  zoom + ImGui::GetIO().MouseWheel);

				bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);
				if(!_panning && mouseDown)
					_lastMousePos = ImGui::GetMousePos();
				_panning = mouseDown;
				if(_panning)
				{
					ImVec2 mousePos = ImGui::GetMousePos();
					rotation.x += (mousePos.y - _lastMousePos.y);
					rotation.y += (mousePos.x - _lastMousePos.x);
					_lastMousePos = ImGui::GetMousePos();
				}
			}

			glm::quat rot = rotationQuat();
			_renderer->position = position + rot * glm::vec3(0, 0, zoom);
			_renderer->rotation = rot;
		}
	}
	ImGui::End();
}

void RenderWindow::lookAt(glm::vec3 pos)
{
	float yaw = atan2(-pos.x, -pos.z);
	float pitch = acos(glm::dot(glm::normalize(position), {0,1,0}));
	rotation = {glm::degrees(pitch) - 90, glm::degrees(yaw)};
}


glm::quat RenderWindow::rotationQuat() const
{
	return glm::angleAxis(glm::radians(rotation.y), glm::vec3(0.0f,1.0f,0.0f)) * glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f,0.0f,0.0f));
}
