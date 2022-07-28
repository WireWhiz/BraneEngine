//
// Created by eli on 5/21/2022.
//

#include "renderWindow.h"
#include <ui/gui.h>
#include "graphics/graphics.h"
#include "graphics/meshRenderer.h"
#include "graphics/material.h"
#include "graphics/renderTarget.h"

#include "ecs/nativeTypes/meshRenderer.h"
#include "assets/assetManager.h"
#include "assets/types/meshAsset.h"
#include "common/ecs/component.h"
#include "ecs/nativeTypes/assetComponents.h"
#include "editor/editorEvents.h"
#include <systems/transforms.h>
#include "backends/imgui_impl_vulkan.h"

RenderWindow::RenderWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{
	EntityManager& em = *Runtime::getModule<EntityManager>();
	graphics::VulkanRuntime& vkr = *Runtime::getModule<graphics::VulkanRuntime>();
	_renderer = vkr.createRenderer<graphics::MeshRenderer>(&vkr, &em);
	_renderer->setClearColor({.2,.2,.2,1});
	_swapChain = vkr.swapChain();
	_sceneRoot = -1;

    auto* defaultMat = new graphics::Material();
    defaultMat->setVertex(vkr.loadShader(0));
    defaultMat->setFragment(vkr.loadShader(1));
    //mat->addTextureDescriptor(vkr.loadTexture(0));
    defaultMat->addBinding(0,sizeof(glm::vec3));
    defaultMat->addBinding(1, sizeof(glm::vec3));
    defaultMat->addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, 0);
    defaultMat->addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, 0);
    vkr.addMaterial(defaultMat);
    em.components().registerComponent(defaultMat->component());

	ui.addEventListener<FocusAssetEvent>("focus asset", [this, &em, &vkr, defaultMat](const FocusAssetEvent* event){
		auto* transformsManager = Runtime::getModule<Transforms>();
		if(dynamic_cast<Assembly*>(event->asset()))
		{
			if(em.entityExists(_sceneRoot))
				transformsManager->destroyRecursive(_sceneRoot);
			_sceneRoot = em.createEntity(ComponentSet({Transform::def()->id}));
			Transform t{};
			em.setComponent(_sceneRoot, t.toVirtual());

			dynamic_cast<Assembly*>(event->asset())->inject(em, _sceneRoot);
		}
        else if(dynamic_cast<MeshAsset*>(event->asset()))
        {
            auto* mesh = dynamic_cast<MeshAsset*>(event->asset());

            if(em.entityExists(_sceneRoot))
                transformsManager->destroyRecursive(_sceneRoot);
            _sceneRoot = em.createEntity(ComponentSet({Transform::def()->id, MeshRendererComponent::def()->id, defaultMat->component()->id}));
            Transform t{};
            em.setComponent(_sceneRoot, t.toVirtual());

            if(mesh->pipelineID == -1)
                vkr.addMesh(mesh);

            MeshRendererComponent mr;
            mr.mesh = mesh->pipelineID;
            mr.materials.push_back(0);
            em.setComponent(_sceneRoot, mr.toVirtual());
        }
	});
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

			if(ImGui::IsWindowHovered() || _panning)
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
