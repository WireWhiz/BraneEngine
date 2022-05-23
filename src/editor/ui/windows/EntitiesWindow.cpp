//
// Created by eli on 5/21/2022.
//

#include "EntitiesWindow.h"

EntitiesWindow::EntitiesWindow(EditorUI& ui) : EditorWindow(ui)
{

}

void EntitiesWindow::draw()
{
	if(ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_None)){
	}
	ImGui::End();
}
