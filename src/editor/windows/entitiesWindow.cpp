//
// Created by eli on 5/21/2022.
//

#include "entitiesWindow.h"
#include "editor/editorEvents.h"
#include "assets/assembly.h"
#include "ui/gui.h"
#include "ecs/entity.h"
#include "systems/transforms.h"
#include "editor/editor.h"
#include "editor/assets/editorAsset.h"
#include "assets/assetManager.h"
#include "editor/assets/assemblyReloadManager.h"

EntitiesWindow::EntitiesWindow(GUI& ui, Editor& editor) : EditorWindow(ui, editor)
{
    _name = "Entities";
	_em = Runtime::getModule<EntityManager>();
	ui.addEventListener<FocusAssetEvent>("focus asset", this, [this](const FocusAssetEvent* event){
        _selected = -1;
		_asset = event->asset();
	});
}

void EntitiesWindow::displayContent()
{
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 13);
    if(_asset && _asset->type() == AssetType::assembly)
    {
	    _asset->json().beginMultiChange();
		displayAssetEntity(_asset->json()["rootEntity"].asUInt());
	    _asset->json().endMultiChange();
    }
    ImGui::PopStyleVar();
    ImGui::Spacing();
}

void EntitiesWindow::displayAssetEntity(uint32_t index)
{
	auto& entity = _asset->json()["entities"][index];
	const bool hasChildren = entity.isMember("children") && entity["children"].size() > 0;

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	if(!hasChildren)
		flags |= ImGuiTreeNodeFlags_Leaf;
    if(index == _selected)
        flags |= ImGuiTreeNodeFlags_Selected;
    std::string name;
    if(entity.isMember("name"))
        name = entity["name"].asString();
    else
        name = "Unnamed " + std::to_string(index);
	bool nodeOpen = ImGui::TreeNodeEx(name.c_str(), flags);
	if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()){
        _ui.sendEvent(std::make_unique<FocusEntityAssetEvent>(index));
        _selected = index;
    }
	bool isRoot = _asset->json()["rootEntity"].asUInt() == _selected;
	if(!isRoot && ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("assetEntity", &index, sizeof(uint32_t));
		ImGui::EndDragDropSource();
	}

	if(ImGui::BeginDragDropTarget())
	{
		if(const ImGuiPayload* droppedEntityPayload  = ImGui::AcceptDragDropPayload("assetEntity"))
		{
			uint32_t droppedIndex = *(uint32_t*)droppedEntityPayload->Data;
			Json::Value droppedEntity = _asset->json()["entities"][droppedIndex];
			if(droppedEntity.isMember("parent"))
			{
				auto& parentEntity = _asset->json()["entities"][droppedEntity["parent"].asUInt()];
				Json::Value newChildren = Json::arrayValue;
				for(auto& child : parentEntity["children"])
				{
					if(child.asUInt() != droppedIndex)
						newChildren.append(child);
				}
				_asset->json().changeValue("entities/" + std::to_string(droppedEntity["parent"].asUInt()) + "/children", newChildren);
			}

			Json::Value newChildren = entity["children"];
			newChildren.append(droppedIndex);
			_asset->json().changeValue("entities/" + std::to_string(index) + "/children", newChildren);
			_asset->json().changeValue("entities/" + std::to_string(droppedIndex) + "/parent", index);
		}
		ImGui::EndDragDropTarget();
	}

	if(nodeOpen)
	{
		if (hasChildren)
		{
			for(auto& child : entity["children"])
				displayAssetEntity(child.asUInt());
		}

		ImGui::TreePop();
	}

}