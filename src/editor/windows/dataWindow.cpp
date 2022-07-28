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
#include <assets/assetManager.h>
#include <assets/assembly.h>
#include "common/ecs/entity.h"

DataWindow::DataWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{
	ui.addEventListener<FocusAssetEvent>("focus asset", [this](const FocusAssetEvent* event){
		_focusedAsset = event->asset();
		_focusMode = FocusMode::asset;
		_focusedAssetEntity = -1;
	});
	ui.addEventListener<FocusEntityAssetEvent>("focus entity asset", [this](const FocusEntityAssetEvent* event){
		_focusedAssetEntity = event->entity();
	});
    ui.addEventListener<FocusEntityEvent>("focus entity", [this](const FocusEntityEvent* event){
        _focusedEntity = event->id();
        _focusMode = FocusMode::entity;
    });
}

void DataWindow::draw()
{
	if(ImGui::Begin("Data Inspector", nullptr, ImGuiWindowFlags_None)){
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
	ImGui::End();
}

void DataWindow::displayAssetData()
{
	if(!_focusedAsset)
		return;

    ImGui::PushFont(_ui.fonts()[1]);
	ImGui::Text("%s", _focusedAsset->name.c_str());
    ImGui::PopFont();
	ImGui::TextDisabled("%s", _focusedAsset->type.toString().c_str());
	switch(_focusedAsset->type.type())
	{
		case AssetType::mesh:
			displayMeshData();
			break;
		case AssetType::assembly:
			displayAssemblyData();
			break;
		default:
			ImGui::Text("Asset Type %s not implemented yet. If you want to edit %s go to the GitHub and open an issue to put pressure on me.", _focusedAsset->type.toString().c_str(), _focusedAsset->name.c_str());
	}
}

void DataWindow::displayAssemblyData()
{
	Assembly* assembly = static_cast<Assembly*>(_focusedAsset);
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
	MeshAsset* mesh = static_cast<MeshAsset*>(_focusedAsset);
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
	auto& entity = static_cast<Assembly*>(_focusedAsset)->entities[_focusedAssetEntity];
	ImGui::Separator();
	ImGui::Text("Entity Index: %u", _focusedAssetEntity);
	ImGui::Separator();
	for (int i = 0; i < entity.components.size(); ++i)
	{
        VirtualVariableWidgets::displayVirtualComponentData(entity.components[i]);
		ImGui::Separator();
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
        if(VirtualVariableWidgets::displayVirtualComponentData(em->getComponent(_focusedEntity, cid)))
            em->markComponentChanged(_focusedEntity, cid);
        ImGui::Separator();
    }
}







