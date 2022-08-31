//
// Created by eli on 5/21/2022.
//

#include "dataWindow.h"
#include "editor/editor.h"
#include "editor/editorEvents.h"
#include "editor/assets/editorAsset.h"
#include "../widgets/virtualVariableWidgets.h"
#include <ui/gui.h>
#include <assets/assembly.h>
#include "assets/assetManager.h"
#include "assets/types/meshAsset.h"
#include <assets/assetID.h>
#include "common/ecs/entity.h"
#include "systems/transforms.h"
#include "assets/types/materialAsset.h"
#include "../widgets/assetSelectWidget.h"
#include "editor/assets/types/editorAssemblyAsset.h"
#include "ecs/nativeTypes/meshRenderer.h"
#include "editor/assets/types/editorMaterialAsset.h"

DataWindow::DataWindow(GUI& ui, Editor& editor) : EditorWindow(ui, editor)
{
    _name = "Data Inspector";
	ui.addEventListener<FocusAssetEvent>("focus asset", this, [this](const FocusAssetEvent* event){
        auto editor = Runtime::getModule<Editor>();
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
	ImGui::Text("%s", _focusedAsset->name().c_str());
    ImGui::PopFont();
	ImGui::TextDisabled("%s", _focusedAsset->type().toString().c_str());
	try{
		switch(_focusedAsset->type().type())
		{
			case AssetType::shader:
				ImGui::Text("Source: %s", _focusedAsset->json()["source"].asCString());
				break;
			case AssetType::mesh:
				displayMeshData();
				break;
			case AssetType::assembly:
				displayAssemblyData();
				break;
			case AssetType::material:
				displayMaterialData();
				break;
			default:
				ImGui::Text("Asset Type %s not implemented yet. If you want to edit %s go to the GitHub and open an issue to put pressure on me.", _focusedAsset->type().toString().c_str(), _focusedAsset->name().c_str());
		}
	}catch(const std::exception& e)
	{
		ImGui::TextColored({1,0,0,1}, "Error displaying asset: %s", e.what());
#ifndef NDEBUG
		throw e;
#endif
	}

}

void DataWindow::displayAssemblyData()
{
	if(_focusedAssetEntity < _focusedAsset->json()["entities"].size())
	{
		displayEntityAssetData();
		return;
	}
	AssetManager& am = *Runtime::getModule<AssetManager>();
	if(ImGui::CollapsingHeader("Dependencies")){
		ImGui::Indent();
		if(ImGui::CollapsingHeader("Materials")){
			ImGui::Indent();
			for(auto& cID : _focusedAsset->json()["dependencies"]["materials"])
			{
				ImGui::Selectable(cID.asCString());
			}
			ImGui::Unindent();
		}
		if(ImGui::CollapsingHeader("Meshes")){
			ImGui::Indent();
			for(auto& mID :  _focusedAsset->json()["dependencies"]["meshes"])
			{
				ImGui::Selectable(mID.asCString());
			}
			ImGui::Unindent();
		}
		ImGui::Unindent();
	}
	ImGui::Text("Entities: %u", _focusedAsset->json()["entities"].size());
	if(ImGui::IsItemHovered())
		ImGui::SetTooltip("Edit in entities window");
}

void DataWindow::displayMeshData()
{
	/*MeshAsset* mesh = static_cast<MeshAsset*>(_focusedAsset->asset());
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
    }*/

}

void DataWindow::displayEntityAssetData()
{
	auto& entityAsset = _focusedAsset->json()["entities"][(Json::ArrayIndex)_focusedAssetEntity];
	ImGui::PushFont(_ui.fonts()[1]);
	if(entityAsset.isMember("name"))
		ImGui::Text("%s", entityAsset["name"].asCString());
	ImGui::PopFont();
	ImGui::TextDisabled("Index: %llu", _focusedAssetEntity);
	ImGui::Separator();
	for (Json::ArrayIndex i = 0; i < entityAsset["components"].size(); ++i)
    {
        auto& component = entityAsset["components"][i];
        if(component["name"].empty())
            continue;
        ImGui::PushID(component["name"].asCString());
        bool displaying = ImGui::CollapsingHeader(component["name"].asCString(), ImGuiTreeNodeFlags_DefaultOpen);
        if(ImGui::BeginPopupContextItem("comp actions"))
        {
            /*bool relationalComponent = component.description() == LocalTransform::def() || component.description() == Children::def();
            bool transform = component.description() == Transform::def();
            bool entityHasChildren = entityAsset.hasComponent(Children::def());
            if(!relationalComponent && !(transform && entityHasChildren))
            {
                if(ImGui::Selectable(ICON_FA_TRASH "delete"))
                    _focusedAsset->removeComponent(_focusedAssetEntity, i);
            }*/
            ImGui::EndPopup();
        }
        /*if(ImGui::BeginDragDropSource())
        {
            ImGui::TextDisabled("%s", component["name"].asCString());
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
        }*/
        if(displaying)
        {
            ImGui::Indent();
			if(component["name"].asString() == MeshRendererComponent::def()->name)
			{
				ImGui::Text("Mesh Index: %d", component["members"][0]["value"].asInt());
				for(Json::ArrayIndex mat = 0; mat < component["members"][1]["value"].size(); ++mat)
				{
					ImGui::PushID(mat);
					Json::ArrayIndex matIndex = component["members"][1]["value"][mat].asUInt();
					ImGui::Text("Material %u (Assembly index %u)", mat, matIndex);
					ImGui::SameLine();
					AssetID matID = _focusedAsset->json()["dependencies"]["materials"].get(matIndex, "null").asString();
					if(AssetSelectWidget::draw(matID, AssetType::material))
					{
						while(matIndex >= _focusedAsset->json()["dependencies"]["materials"].size())
							_focusedAsset->json().data()["dependencies"]["materials"].append("null");

						_focusedAsset->json().changeValue("dependencies/materials/" + std::to_string(matIndex), matID.string());
						_editor.reloadAsset(_focusedAsset);
						if(!matID.isNull())
							Runtime::getModule<AssetManager>()->fetchAsset<MaterialAsset>(matID).then([this](auto* m){_ui.sendEvent<EntityAssetReloadEvent>(_focusedAssetEntity);});
						else
							_ui.sendEvent<EntityAssetReloadEvent>(_focusedAssetEntity);
					}
					ImGui::PopID();
				}
			}
			else
			{
				Json::Value data = component;
				auto res = VirtualVariableWidgets::displayAssetComponentData(data, _focusedAsset->json().data());
				if(res != UiChangeType::none)
				{
					_focusedAsset->json().changeValue("entities/" + std::to_string(_focusedAssetEntity) + "/components/" + std::to_string(i), data, res == UiChangeType::finished);
					if(res == UiChangeType::finished)
						_editor.reloadAsset(_focusedAsset);
				}
			}
            ImGui::Unindent();
        }
        ImGui::PopID();
	}
    if(ImGui::Button("Add Component", {ImGui::GetContentRegionAvail().x, 0}))
    {
        ImGui::OpenPopup("add component");
    }
    if(ImGui::BeginPopup("add component"))
    {
        ImGui::TextDisabled("Can't add components yet.. because I'm lazy. Open an Issue and yell at me.");
        ImGui::EndPopup();
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

void DataWindow::displayMaterialData()
{
    auto* material = static_cast<EditorMaterialAsset*>(_focusedAsset.get());
    AssetID vertexID = material->json()["vertexShader"].asString();
    if(AssetSelectWidget::draw(vertexID, AssetType::shader))
    {
	    material->json().changeValue("vertexShader", vertexID.string());
	    _editor.reloadAsset(_focusedAsset);
    };
    ImGui::SameLine();
    ImGui::Text("Vertex Shader");
    AssetID fragmentID = material->json()["fragmentShader"].asString();
    if(AssetSelectWidget::draw(fragmentID, AssetType::shader))
    {
	    material->json().changeValue("fragmentShader", fragmentID.string());
		_editor.reloadAsset(_focusedAsset);
    }
    ImGui::SameLine();
    ImGui::Text("Fragment Shader");
    ImGui::Spacing();
    ImGui::TextDisabled("Custom property components coming eventually");
}







