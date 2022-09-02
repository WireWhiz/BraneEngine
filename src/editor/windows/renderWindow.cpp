//
// Created by eli on 5/21/2022.
//

#include "renderWindow.h"
#include "ui/gui.h"
#include "../editor.h"
#include "../editorEvents.h"
#include "../assets/editorAsset.h"
#include "graphics/graphics.h"
#include "graphics/meshRenderer.h"
#include "graphics/material.h"
#include "graphics/renderTarget.h"

#include "assets/assetManager.h"
#include "assets/types/meshAsset.h"
#include "ecs/nativeTypes/assetComponents.h"
#include <systems/transforms.h>
#include "backends/imgui_impl_vulkan.h"
#include "editor/assets/types/editorAssemblyAsset.h"

class RenderWindowAssetReady : public GUIEvent
{
	AssetID _id;
public:
	RenderWindowAssetReady(const AssetID& id) : _id(id), GUIEvent("render window asset ready"){};
	const AssetID& id() const {return _id;}
};

RenderWindow::RenderWindow(GUI& ui, Editor& editor) : EditorWindow(ui, editor)
{
    _name = "Render";
	EntityManager& em = *Runtime::getModule<EntityManager>();
	graphics::VulkanRuntime& vkr = *Runtime::getModule<graphics::VulkanRuntime>();
	_renderer = vkr.createRenderer<graphics::MeshRenderer>(&vkr, &em);
	_renderer->setClearColor({.2,.2,.2,1});
	_swapChain = vkr.swapChain();
    _ui.addEventListener<FocusAssetEvent>("focus asset", this, [this, &em](const FocusAssetEvent* event){
        if(!_entities.empty())
        {
			for(auto& e : _entities)
				em.destroyEntity(e);
			_entities.resize(0);
		}
		auto* am = Runtime::getModule<AssetManager>();
        _focusedAsset = event->asset();
		_focusedAssetEntity = -1;
		if(_focusedAsset->json().data().isMember("entities"))
		{
			if(dynamic_cast<EditorAssemblyAsset*>(_focusedAsset.get()))
			{
				am->fetchAsset<Assembly>(_focusedAsset->json()["id"].asString()).then([this](Assembly* assembly){
					_ui.sendEvent(std::make_unique<RenderWindowAssetReady>(assembly->id));
				}).onError([this](const std::string& error){
					if(_focusedAsset)
						Runtime::warn("Could not load " + _focusedAsset->name() + " to render: " + error);
					else
						Runtime::warn("Could not load asset to render: " + error);
				});
			}
		}
    });
	_ui.addEventListener<RenderWindowAssetReady>("render window asset ready", this, [this, &em](const RenderWindowAssetReady* event){
		auto* am = Runtime::getModule<AssetManager>();
		Assembly* assembly = am->getAsset<Assembly>(event->id());
		EntityID root = em.createEntity();
		em.addComponent<Transform>(root);
		Transform t;
		t.value = glm::translate(glm::mat4(1), {0, 0, 0});
		em.setComponent(root, t.toVirtual());
		_entities = assembly->inject(em, root);
		_entities.push_back(root);

		//Second for index testing
		EntityID root2 = em.createEntity();
		em.addComponent<Transform>(root2);
		t.value = glm::translate(glm::mat4(1), {2, 0, 0});
		em.setComponent(root2, t.toVirtual());
		auto entities2 = assembly->inject(em, root2);
		_entities.insert(_entities.end(), entities2.begin(), entities2.end());
		_entities.push_back(root);
	});
	_ui.addEventListener<EntityAssetReloadEvent>("entity asset reload", this, [this](const EntityAssetReloadEvent* event){
		if(_focusedAsset && event->entity() < _entities.size())
		{
			dynamic_cast<EditorAssemblyAsset*>(_focusedAsset.get())->updateEntity(event->entity(), _entities);
		}
	});
    _ui.addEventListener<FocusEntityAssetEvent>("focus entity asset", this, [this](const FocusEntityAssetEvent* event){
        _focusedAssetEntity = event->entity();
        _focusedEntity = _entities[event->entity()];
    });
    /*_ui.addEventListener<FocusEntityEvent>("focus entity", this, [this](const FocusEntityEvent* event){
        if(_focusedAsset)
            _focusedAsset->setPreviewEntities(false);
        _focusedAsset = nullptr;
        _focusedEntity = event->id();
    });*/
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
					Json::Value components = _focusedAsset->json()["entities"][(Json::ArrayIndex)_focusedAssetEntity]["components"];
					assert(components != Json::nullValue);
					if(em->hasComponent<TRS>(_focusedEntity))
					{
						for(auto& member : components)
						{
							if(member["id"] == TRS::def()->asset->id.string())
							{
								member = EditorAssemblyAsset::componentToJson(em->getComponent<TRS>(_focusedEntity)->toVirtual());
								break;
							}
						}
					}
	                if(em->hasComponent<Transform>(_focusedEntity))
	                {
		                for(auto& member : components)
		                {
			                if(member["id"] == Transform::def()->asset->id.string())
			                {
				                member = EditorAssemblyAsset::componentToJson(em->getComponent<Transform>(_focusedEntity)->toVirtual());
				                break;
			                }
		                }
	                }
                    _focusedAsset->json().changeValue("entities/" + std::to_string(_focusedAssetEntity) + "/components", components);
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
