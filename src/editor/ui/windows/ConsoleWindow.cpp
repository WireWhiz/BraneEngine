//
// Created by eli on 5/21/2022.
//

#include "ConsoleWindow.h"

ConsoleWindow::ConsoleWindow(EditorUI& ui) : EditorWindow(ui)
{

}

void ConsoleWindow::draw()
{
	if(ImGui::Begin("Console", nullptr, ImGuiWindowFlags_None)){
	}
	ImGui::End();
}
