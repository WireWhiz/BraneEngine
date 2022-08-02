//
// Created by eli on 5/21/2022.
//

#include "entitiesWindow.h"
#include "editor/editorEvents.h"
#include "assets/assembly.h"
#include "ui/gui.h"
#include "ecs/entity.h"
#include "systems/transforms.h"
#include "../assetEditorContext.h"

EntitiesWindow::EntitiesWindow(GUI& ui) : GUIWindow(ui)
{
    _name = "Entities";
	_em = Runtime::getModule<EntityManager>();
	ui.addEventListener<FocusAssetEvent>("focus asset", this, [this](const FocusAssetEvent* event){
        _selected = -1;
        _assetCtx = event->asset();
	});
}

void EntitiesWindow::displayContent()
{
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 13);
    if(_assetCtx && dynamic_cast<Assembly*>(_assetCtx->asset()))
    {
        auto* assembly = dynamic_cast<Assembly*>(_assetCtx->asset());
        for (size_t i = 0; i < assembly->entities.size(); ++i)
        {
            if(assembly->entities[i].hasComponent(LocalTransform::def()))
                continue;
            displayAssemblyEntities(assembly, i);
        }
    }
    ImGui::PopStyleVar();
    ImGui::Spacing();
}

void EntitiesWindow::displayAssemblyEntities(Assembly* assembly, size_t entIndex)
{
	auto& entity = assembly->entities[entIndex];
	const bool hasChildren = entity.hasComponent(Children::def());

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	if(!hasChildren)
		flags |= ImGuiTreeNodeFlags_Leaf;
    if(entIndex == _selected)
        flags |= ImGuiTreeNodeFlags_Selected;
    std::string name;
    if(entity.hasComponent(EntityName::def()))
        name = *entity.getComponent(EntityName::def())->getVar<std::string>(0);
    else
        name = "Unnamed " + std::to_string(entIndex);
	bool nodeOpen = ImGui::TreeNodeEx(name.c_str(), flags);
	if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()){
        _ui.sendEvent(std::make_unique<FocusEntityAssetEvent>(entIndex));
        _selected = entIndex;
    }
	if(nodeOpen)
	{
		if (hasChildren)
		{
			auto* childrenComponent = Children::fromVirtual(*entity.getComponent(Children::def()));
			for(auto& childIndex : childrenComponent->children)
			{
				displayAssemblyEntities(assembly, childIndex);
			}
		}

		ImGui::TreePop();
	}
}