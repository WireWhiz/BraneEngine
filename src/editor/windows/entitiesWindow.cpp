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
	    displayAssetEntity(_asset->json()["rootEntity"].asUInt());
    }
    ImGui::PopStyleVar();
    ImGui::Spacing();
}

void EntitiesWindow::displayAssetEntity(unsigned int index)
{
	auto& entity = _asset->json()["entities"][index];
	const bool hasChildren = entity.isMember("children");

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