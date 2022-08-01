//
// Created by eli on 5/21/2022.
//

#include "renderWindow.h"
#include "ui/gui.h"
#include "../editorEvents.h"
#include "../assetEditorContext.h"
#include "graphics/graphics.h"
#include "graphics/meshRenderer.h"
#include "graphics/material.h"
#include "graphics/renderTarget.h"

#include "ecs/nativeTypes/meshRenderer.h"
#include "assets/assetManager.h"
#include "assets/types/meshAsset.h"
#include "common/ecs/component.h"
#include "ecs/nativeTypes/assetComponents.h"
#include <systems/transforms.h>
#include "backends/imgui_impl_vulkan.h"
#include <ImGuizmo.h>

RenderWindow::RenderWindow(GUI& ui) : GUIWindow(ui)
{
    _name = "Render";
	EntityManager& em = *Runtime::getModule<EntityManager>();
	graphics::VulkanRuntime& vkr = *Runtime::getModule<graphics::VulkanRuntime>();
	_renderer = vkr.createRenderer<graphics::MeshRenderer>(&vkr, &em);
	_renderer->setClearColor({.2,.2,.2,1});
	_swapChain = vkr.swapChain();
    _ui.addEventListener<FocusAssetEvent>("focus asset", this, [this](const FocusAssetEvent* event){
        _focusedAsset = event->asset();
        _focusedEntity.version = -1;
    });
    _ui.addEventListener<FocusEntityAssetEvent>("focus entity asset", this, [this](const FocusEntityAssetEvent* event){
        _focusedEntity = _focusedAsset->entities()[event->entity()];
    });
    _ui.addEventListener<FocusEntityEvent>("focus entity", this, [this](const FocusEntityEvent* event){
        _focusedAsset = nullptr;
        _focusedEntity = event->id();
    });
}

RenderWindow::~RenderWindow()
{
    graphics::VulkanRuntime& vkr = *Runtime::getModule<graphics::VulkanRuntime>();
    vkDeviceWaitIdle(graphics::device->get());
    vkr.removeRenderer(_renderer);
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

void RenderWindow::displayContent()
{
    auto window = ImGui::GetContentRegionAvail();
    _windowSize = {static_cast<uint32_t>(glm::floor(glm::max((float)0,window.x))), static_cast<uint32_t>(glm::floor(glm::max((float)0,window.y)))};
    if(_windowSize.width != 0 && _windowSize.height != 0)
    {
        if(_texture && (_windowSize.width != _texture->size().width ||
                        _windowSize.height != _texture->size().height))
        {
            _queueReload = true;
        }

        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, window.x, window.y);
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
        auto* em = Runtime::getModule<EntityManager>();
        if(em->entityExists(_focusedEntity) && em->hasComponent<Transform>(_focusedEntity))
        {
            glm::mat4 vt = _renderer->transformMatrix();
            glm::mat4 pt = _renderer->perspectiveMatrix();
            //Undo inversion since ImGuizmo won't expect it
            pt[1][1] *= -1;

            auto objectTransform = Transforms::getGlobalTransform(_focusedEntity, *em);

            ImGuizmo::Manipulate((float*)&vt, (float*)&pt, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, (float*)&objectTransform);
            if(ImGuizmo::IsUsing())
            {
                _manipulating = true;
                Transforms::setGlobalTransform(_focusedEntity, objectTransform, *em);
            }
            else if(_manipulating)
            {
                _manipulating = false;
                //Event on manipulation change
            }
        }

        glm::quat rot = rotationQuat();
        _renderer->position = position + rot * glm::vec3(0, 0, zoom);
        _renderer->rotation = rot;
    }
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
