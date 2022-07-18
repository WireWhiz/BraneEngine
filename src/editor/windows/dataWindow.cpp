//
// Created by eli on 5/21/2022.
//

#include "dataWindow.h"
#include "editorEvents.h"
#include <ui/gui.h>
#include <assets/assembly.h>
#include <assets/assetManager.h>

DataWindow::DataWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{
	ui.addEventListener<FocusAssetEvent>("focus asset", [this](FocusAssetEvent* event){
		_focusedAsset = event->asset();
		_focusMode = FocusMode::asset;
		_focusedAssetEntity = -1;
	});
	ui.addEventListener<FocusEntityAssetEvent>("focus entity asset", [this](FocusEntityAssetEvent* event){
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
	ImGui::Text("%s", _focusedAsset->name.c_str());
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
		displayVirtualComponentData(entity.components[i]);
		ImGui::Separator();
	}
}

void DataWindow::displayEntityData()
{

}

void DataWindow::displayVirtualComponentData(VirtualComponentView component)
{
	ImGui::Text("%s", component.description()->name.c_str());
	for(size_t i = 0; i < component.description()->members().size(); ++i)
	{
		auto& member = component.description()->members()[i];
		const std::string& name = component.description()->asset->memberNames()[i];
		if(!name.empty()){
			displayVirtualVariable(name.c_str(), member.type, component.data() + member.offset);
			ImGui::Spacing();
		}
	}
}

void DataWindow::displayVirtualVariable(const char* name, VirtualType::Type type, byte* data)
{
	assert(name);
	switch(type)
	{
		case VirtualType::virtualUnknown:
			ImGui::Text("Can't edit virtual unknowns, how are you seeing this?");
			break;
		case VirtualType::virtualBool:
			ImGui::Checkbox(name, (bool*)data);
			break;
		case VirtualType::virtualInt:
			ImGui::InputScalar(name, ImGuiDataType_S32, (void*)data, NULL, NULL, "%d", 0);
			break;
		case VirtualType::virtualInt64:
			ImGui::InputScalar(name, ImGuiDataType_S64, (void*)data, NULL, NULL, "%d", 0);
			break;
		case VirtualType::virtualUInt:
			ImGui::InputScalar(name, ImGuiDataType_U32, (void*)data, NULL, NULL, "%u", 0);
			break;
		case VirtualType::virtualUInt64:
			ImGui::InputScalar(name, ImGuiDataType_U64, (void*)data, NULL, NULL, "%u", 0);
			break;
		case VirtualType::virtualFloat:
			ImGui::InputScalar(name, ImGuiDataType_Float, (void*)data, NULL, NULL, "%.3f", 0);
			break;
		case VirtualType::virtualString:
			ImGui::InputText(name, (std::string*)data);
			break;
		case VirtualType::virtualAssetID:
		{
			std::string id = ((AssetID*)data)->string();
			ImGui::InputText(name, &id);
			if(ImGui::IsItemDeactivatedAfterEdit())
				((AssetID*)data)->parseString(id);
		}
			break;
		case VirtualType::virtualVec3:
			ImGui::InputFloat3(name, (float*)data);
			break;
		case VirtualType::virtualVec4:
			ImGui::InputFloat4(name, (float*)data);
			break;
		case VirtualType::virtualQuat:
			ImGui::InputFloat4(name, (float*)data);
			if(ImGui::IsItemDeactivatedAfterEdit())
				*(glm::quat*)data = glm::normalize(*(glm::quat*)data);
			break;
		case VirtualType::virtualMat4:
		{
			glm::mat4& mat = *(glm::mat4*)data;
			ImGui::PushID(name);
			ImGui::InputFloat4(name, &mat[0][0]);
			ImGui::InputFloat4("##MatValue2", &mat[1][0]);
			ImGui::InputFloat4("##MatValue3", &mat[2][0]);
			ImGui::InputFloat4("##MatValue4", &mat[3][0]);
			ImGui::PopID();
		}
			break;
		case VirtualType::virtualFloatArray:
		{
			inlineFloatArray& array = *(inlineFloatArray*)data;
			ImGui::Text("%s (TODO: Array editing)", name);
			for(auto f : array)
			{
				ImGui::TextDisabled("%f", f);
			}
		}
			break;
		case VirtualType::virtualIntArray:
		{
			inlineIntArray& array = *(inlineIntArray*)data;
			ImGui::Text("%s (TODO: Array editing)", name);
			for(auto f : array)
			{
				ImGui::TextDisabled("%d", f);
			}
		}
			break;
		case VirtualType::virtualUIntArray:
		{
			inlineUIntArray& array = *(inlineUIntArray*)data;
			ImGui::Text("%s (TODO: Array editing)", name);
			for(auto f : array)
			{
				ImGui::TextDisabled("%u", f);
			}
		}
			break;
	}
}





