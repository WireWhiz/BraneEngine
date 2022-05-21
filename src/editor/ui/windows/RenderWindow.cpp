//
// Created by eli on 5/21/2022.
//

#include "RenderWindow.h"

RenderWindow::RenderWindow(EditorUI& ui) : EditorWindow(ui)
{

}

void RenderWindow::draw()
{
	if(ImGui::Begin("Render")){
		ImGui::End();
	}
}
