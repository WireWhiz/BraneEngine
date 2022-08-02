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
        _focusedAssetEntity = event->entity();
        _focusedEntity = _focusedAsset->entities()[event->entity()];
    });
    _ui.addEventListener<FocusEntityEvent>("focus entity", this, [this](const FocusEntityEvent* event){
        _focusedAsset = nullptr;
        _focusedEntity = event->id();
    });
    ImGuizmo::AllowAxisFlip(false); // Maybe add this to the config at some point
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
    const bool hovered = ImGui::IsWindowHovered();
    ImGui::BeginDisabled(_gizmoOperation == ImGuizmo::OPERATION::TRANSLATE);
    if(ImGui::Button(ICON_FA_UP_DOWN_LEFT_RIGHT) || (hovered && ImGui::IsKeyPressed(ImGuiKey_W, false)))
        _gizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
    ImGui::EndDisabled();

    ImGui::SameLine(0,0);
    ImGui::BeginDisabled(_gizmoOperation == ImGuizmo::OPERATION::ROTATE);
    if(ImGui::Button(ICON_FA_ROTATE) || (hovered && ImGui::IsKeyPressed(ImGuiKey_E, false)))
        _gizmoOperation = ImGuizmo::OPERATION::ROTATE;
    ImGui::EndDisabled();

    ImGui::SameLine(0,0);
    ImGui::BeginDisabled(_gizmoOperation == ImGuizmo::OPERATION::SCALE);
    if(ImGui::Button(ICON_FA_ARROWS_TO_DOT) || (hovered && ImGui::IsKeyPressed(ImGuiKey_R, false)))
        _gizmoOperation = ImGuizmo::OPERATION::SCALE;
    ImGui::EndDisabled();

    ImGui::SameLine(0, 13);
    if(ImGui::Button(_gizmoMode == ImGuizmo::MODE::WORLD ? ICON_FA_GLOBE : ICON_FA_OBJECT_GROUP)  || (hovered && ImGui::IsKeyPressed(ImGuiKey_Q, false)))
        _gizmoMode = _gizmoMode == ImGuizmo::MODE::WORLD ? ImGuizmo::MODE::LOCAL : ImGuizmo::MODE::WORLD;


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

        if(_texture){

            ImGui::Image(_imGuiBindings[_swapChain->currentFrame()], window);
            auto imgPos = ImGui::GetItemRectMin();
            auto imgSize = ImGui::GetItemRectSize();
            ImGuizmo::SetRect(imgPos.x, imgPos.y, imgSize.x, imgSize.y);
        }

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

            if(ImGui::IsKeyDown(ImGuiKey_ModCtrl))
            {
                if(ImGui::IsKeyPressed(ImGuiKey_Y) || (ImGui::IsKeyDown(ImGuiKey_ModShift) && ImGui::IsKeyPressed(ImGuiKey_Z)))
                    _focusedAsset->redo();
                else if(ImGui::IsKeyPressed(ImGuiKey_Z))
                    _focusedAsset->undo();
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

            ImGuizmo::Manipulate((float*)&vt, (float*)&pt, _gizmoOperation, (_gizmoOperation != ImGuizmo::OPERATION::SCALE) ? _gizmoMode : ImGuizmo::MODE::LOCAL, (float*)&objectTransform);
            if(ImGuizmo::IsUsing())
            {
                _manipulating = true;
                Transforms::setGlobalTransform(_focusedEntity, objectTransform, *em);
            }
            else if(_manipulating)
            {
                _manipulating = false;
                if(_focusedAsset)
                {
                    _focusedAsset->updateEntity(_focusedAssetEntity);
                }
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
