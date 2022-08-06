//
// Created by eli on 5/21/2022.
//

#include "dataWindow.h"
#include "editor/editorEvents.h"
#include "../widgets/virtualVariableWidgets.h"
#include <ui/gui.h>
#include <assets/assembly.h>
#include "assets/assetManager.h"
#include "assets/types/meshAsset.h"
#include <assets/assetID.h>
#include "common/ecs/entity.h"
#include "../assetEditorContext.h"
#include "systems/transforms.h"

DataWindow::DataWindow(GUI& ui) : GUIWindow(ui)
{
    _name = "Data Inspector";
	ui.addEventListener<FocusAssetEvent>("focus asset", this, [this](const FocusAssetEvent* event){
		_focusedAsset = event->asset();
		_focusMode = FocusMode::asset;
		_focusedAssetEntity = -1;
	});
	ui.addEventListener<FocusEntityAssetEvent>("focus entity asset", this, [this](const FocusEntityAssetEvent* event){
		_focusedAssetEntity = event->entity();
	});
    ui.addEventListener<FocusEntityEvent>("focus entity", this, [this](const FocusEntityEvent* event){
        _focusedEntity = event->id();
        _focusMode = FocusMode::entity;
    });
}

void DataWindow::displayContent()
{
    switch(_focusMode)
    {
        case FocusMode::asset:
            displayAssetData();
            break;
        case FocusMode::entity:
            displayEntityData();
            break;
        default:
            ImGui::TextDisabled("No data to display");
    }
}

void DataWindow::displayAssetData()
{
	if(!_focusedAsset)
		return;

    ImGui::PushFont(_ui.fonts()[1]);
	ImGui::Text("%s", _focusedAsset->asset()->name.c_str());
    ImGui::PopFont();
	ImGui::TextDisabled("%s", _focusedAsset->asset()->type.toString().c_str());
	switch(_focusedAsset->asset()->type.type())
	{
		case AssetType::mesh:
			displayMeshData();
			break;
		case AssetType::assembly:
			displayAssemblyData();
			break;
		default:
			ImGui::Text("Asset Type %s not implemented yet. If you want to edit %s go to the GitHub and open an issue to put pressure on me.", _focusedAsset->asset()->type.toString().c_str(), _focusedAsset->asset()->name.c_str());
	}
}

void DataWindow::displayAssemblyData()
{
	Assembly* assembly = static_cast<Assembly*>(_focusedAsset->asset());
	if(_focusedAssetEntity < assembly->entities.size())
	{
		displayEntityAssetData();
		return;
	}
	AssetManager& am = *Runtime::getModule<AssetManager>();
	if(ImGui::CollapsingHeader("Dependencies")){
		ImGui::Indent(16);
		if(ImGui::CollapsingHeader("Components")){
			ImGui::Indent(16);
			for(auto& cID : assembly->components)
			{
				ComponentAsset* asset = am.getAsset<ComponentAsset>(cID);
				ImGui::PushID(asset->name.c_str());
				ImGui::Selectable(asset->name.c_str());
				if(ImGui::IsItemHovered())
					ImGui::SetTooltip("ID: %s", cID.string().c_str());
				ImGui::PopID();
			}
			ImGui::Unindent(16);
		}
		if(ImGui::CollapsingHeader("Meshes")){
			ImGui::Indent(16);
			for(auto& mID : assembly->meshes)
			{
				MeshAsset* asset = am.getAsset<MeshAsset>(mID);
				ImGui::Selectable(asset->name.c_str());
				if(ImGui::IsItemHovered())
					ImGui::SetTooltip("ID: %s", mID.string().c_str());
			}
			ImGui::Unindent(16);
		}
		ImGui::Unindent(16);
	}
	ImGui::Text("Entities: %u", assembly->entities.size());
	if(ImGui::IsItemHovered())
		ImGui::SetTooltip("Edit in entities window");
}

void DataWindow::displayMeshData()
{
	MeshAsset* mesh = static_cast<MeshAsset*>(_focusedAsset->asset());
	ImGui::Text("Primitives %u", mesh->primitiveCount());
	uint32_t vertices = 0;
	uint32_t tris = 0;
	for (int i = 0; i < mesh->primitiveCount(); ++i)
	{
			vertices += mesh->vertexCount(i);
			tris += mesh->indexCount(i);
	}
	tris /= 3;
	ImGui::Text("Total vertices: %u", vertices);
	ImGui::Text("Total triangles: %u", tris);
    for (size_t i = 0; i < mesh->primitiveCount(); ++i)
    {
        ImGui::Separator();
        ImGui::PushID(i);
        ImGui::PushFont(_ui.fonts()[1]);
        ImGui::Text("Primitive %d", i);
        ImGui::PopFont();
        ImGui::Text("Vertices: %u", mesh->vertexCount(i));
        ImGui::Text("Triangles: %u", mesh->indexCount(i) / 3);
        if(ImGui::CollapsingHeader("Vertices"))
        {
            ImGui::Indent(16);
            const byte* poses = &mesh->packedData()[mesh->attributeOffset(i, "POSITION")];
            for(size_t j = 0; j < mesh->vertexCount(i); ++j)
            {
                ImGui::InputFloat3(std::to_string(j).c_str(), (float*)(poses + j * sizeof(float) * 3), "%.3f", ImGuiInputTextFlags_ReadOnly);
            }
            ImGui::Unindent(16);
        }
        ImGui::PopID();
    }

}

void DataWindow::displayEntityAssetData()
{
	auto& entityAsset = static_cast<Assembly*>(_focusedAsset->asset())->entities[_focusedAssetEntity];
	ImGui::Separator();
	ImGui::Text("Entity Index: %lu", _focusedAssetEntity);
	ImGui::Separator();
    auto* em = Runtime::getModule<EntityManager>();
	for (size_t i = 0; i < entityAsset.components.size(); ++i)
    {
        VirtualComponentView component = em->getComponent(_focusedAsset->entities()[_focusedAssetEntity].id, entityAsset.components[i].description()->id);
        if(component.description()->name.empty())
            continue;
        ImGui::PushID(i);
        bool displaying = ImGui::CollapsingHeader(component.description()->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
        if(ImGui::BeginPopupContextItem("comp actions"))
        {
            bool relationalComponent = component.description() == LocalTransform::def() || component.description() == Children::def();
            bool transform = component.description() == Transform::def();
            bool entityHasChildren = entityAsset.hasComponent(Children::def());
            if(!relationalComponent && !(transform && entityHasChildren))
            {
                if(ImGui::Selectable(ICON_FA_TRASH "delete"))
                    _focusedAsset->removeComponent(_focusedAssetEntity, i);
            }
            ImGui::EndPopup();
        }
        if(ImGui::BeginDragDropSource())
        {
            ImGui::TextDisabled("%s", component.description()->name.c_str());
            DraggedComponent dragData{(Assembly*)_focusedAsset->asset(), _focusedAssetEntity, i};
            ImGui::SetDragDropPayload("component", &dragData, sizeof(DraggedComponent));
            ImGui::EndDragDropSource();
        }
        if(ImGui::BeginDragDropTarget())
        {
            if(const ImGuiPayload* p = ImGui::AcceptDragDropPayload("component"))
            {
                auto dc = static_cast<DraggedComponent*>(p->Data);
                auto& srcComponents = dc->asset->entities[dc->entity].components;
                VirtualComponent source = srcComponents[dc->componentIndex];
                srcComponents.erase(srcComponents.begin() + dc->componentIndex);
                auto& destComponents = entityAsset.components;
                // The min is to account for if we're trying to move the component to the end, and just removed it from the same list.
                destComponents.insert(destComponents.begin() + std::min<uint32_t>(destComponents.size(), i), source);
            }
            ImGui::EndDragDropTarget();
        }
        if(displaying)
        {
            ImGui::Indent(13);
            auto res = VirtualVariableWidgets::displayVirtualComponentData(component);
            if((uint8_t)res > 0)
                em->markComponentChanged(_focusedAsset->entities()[_focusedAssetEntity].id, entityAsset.components[i].description()->id);
            if(res == UiChangeType::finished)
                _focusedAsset->updateEntity(_focusedAssetEntity);
            ImGui::Unindent(13);
        }
        ImGui::PopID();
	}
    if(ImGui::Button("Add Component", {ImGui::GetContentRegionAvail().x, 0}))
    {
        ImGui::OpenPopup("add component");
    }
    if(ImGui::BeginPopup("add component"))
    {
        ImGui::TextDisabled("Can't add components yet.. need to make server calls for this");
        ImGui::EndPopup();
    }
    if(ImGui::IsWindowHovered() && ImGui::IsKeyDown(ImGuiKey_ModCtrl))
    {
        if(ImGui::IsKeyPressed(ImGuiKey_Y) || (ImGui::IsKeyDown(ImGuiKey_ModShift) && ImGui::IsKeyPressed(ImGuiKey_Z)))
            _focusedAsset->redo();
        else if(ImGui::IsKeyPressed(ImGuiKey_Z))
            _focusedAsset->undo();
    }
}

void DataWindow::displayEntityData()
{

    ImGui::Text("Entity ID: %u", _focusedEntity.id);
    ImGui::TextDisabled("Version: %u", _focusedEntity.version);
    ImGui::Separator();
    auto* em = Runtime::getModule<EntityManager>();
    if(!em->entityExists(_focusedEntity))
    {
        ImGui::TextColored({1,0,0,1}, "Entity Destroyed!");
        return;
    }

    auto& components = em->getEntityArchetype(_focusedEntity)->components();
    for (auto cid : components)
    {
        if((int)VirtualVariableWidgets::displayVirtualComponentData(em->getComponent(_focusedEntity, cid)) > 0)
            em->markComponentChanged(_focusedEntity, cid);
        ImGui::Separator();
    }
}







