//
// Created by eli on 5/21/2022.
//

#include "consoleWindow.h"

ConsoleWindow::ConsoleWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{

}

void ConsoleWindow::draw()
{
	if(ImGui::Begin("Console", nullptr, ImGuiWindowFlags_None)){
	}
	ImGui::End();
}
