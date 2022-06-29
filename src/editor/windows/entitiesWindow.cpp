//
// Created by eli on 5/21/2022.
//

#include "entitiesWindow.h"

EntitiesWindow::EntitiesWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{

}

void EntitiesWindow::draw()
{
	if(ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_None)){
	}
	ImGui::End();
}
