//
// Created by eli on 5/21/2022.
//

#include "dataWindow.h"
#include "editor/editorEvents.h"
#include "../widgets/virtualVariableWidgets.h"
#include <ui/gui.h>
#include <assets/assembly.h>
#include <assets/assetManager.h>

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
	ImGui::Text("Vertices: %u", vertices);
	ImGui::Text("Tris: %u", tris);
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

}







