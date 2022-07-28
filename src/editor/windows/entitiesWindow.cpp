//
// Created by eli on 5/21/2022.
//

#include "entitiesWindow.h"
#include "editor/editorEvents.h"
#include "assets/assembly.h"
#include "ui/gui.h"
#include "ecs/entity.h"
#include "systems/transforms.h"

EntitiesWindow::EntitiesWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{
	_em = Runtime::getModule<EntityManager>();
	ui.addEventListener<FocusAssetEvent>("focus asset", [this](const FocusAssetEvent* event){
		_assembly = dynamic_cast<Assembly*>(event->asset());
	});
}

void EntitiesWindow::draw()
{
	if(ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_None)){
		if(_assembly && ImGui::CollapsingHeader("Asset Entities"))
		{
			ImGui::Indent(16);
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 13);
			for (size_t i = 0; i < _assembly->entities.size(); ++i)
			{
				if(_assembly->entities[i].hasComponent(LocalTransform::def()))
					continue;
				displayAssemblyEntities(_assembly, i);
			}
			ImGui::PopStyleVar();
			ImGui::Unindent(16);
		}
		if(ImGui::CollapsingHeader("Scene Entities"))
		{
			_em->systems().runUnmanagedSystem("display entities", [this](SystemContext* ctx){
				ImGui::Indent(16);
				ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 13);
				ComponentFilter namedEntities(ctx);
                namedEntities.addComponent(EntityIDComponent::def()->id, ComponentFilterFlags_Const);
                namedEntities.addComponent(EntityName::def()->id, ComponentFilterFlags_Const);
				_em->getEntities(namedEntities).forEachNative([this](byte** components){
					auto* id = EntityIDComponent::fromVirtual(components[0]);
                    auto* name = EntityName::fromVirtual(components[1]);
					if(ImGui::Selectable(name->name.c_str()))
                    {
                        _ui.sendEvent(std::make_unique<FocusEntityEvent>(id->id));
                    }
				});
                ComponentFilter unnamedEntities(ctx);
                unnamedEntities.addComponent(EntityIDComponent::def()->id, ComponentFilterFlags_Const);
                unnamedEntities.addComponent(EntityName::def()->id, ComponentFilterFlags_Exclude);
                _em->getEntities(unnamedEntities).forEachNative([this](byte** components){
                    auto* id = EntityIDComponent::fromVirtual(components[0]);
                    if(ImGui::Selectable(("Unnamed " + std::to_string(id->id)).c_str()))
                    {
                        _ui.sendEvent(std::make_unique<FocusEntityEvent>(id->id));
                    }
                });
				ImGui::PopStyleVar();
				ImGui::Unindent(16);
			});
		}
	}
	ImGui::End();
}

void EntitiesWindow::displayAssemblyEntities(Assembly* assembly, size_t entIndex)
{
	auto& entity = assembly->entities[entIndex];
	const bool hasChildren = entity.hasComponent(Children::def());

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	if(!hasChildren)
		flags |= ImGuiTreeNodeFlags_Leaf;
    std::string name;
    if(entity.hasComponent(EntityName::def()))
        name = *entity.getComponent(EntityName::def())->getVar<std::string>(0);
    else
        name = "Unnamed " + std::to_string(entIndex);
	bool nodeOpen = ImGui::TreeNodeEx(name.c_str(), flags);
	if(ImGui::IsItemClicked())
		_ui.sendEvent(std::make_unique<FocusEntityAssetEvent>(entIndex));
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