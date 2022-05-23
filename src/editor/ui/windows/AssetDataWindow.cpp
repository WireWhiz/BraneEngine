//
// Created by eli on 5/21/2022.
//

#include "AssetDataWindow.h"

AssetDataWindow::AssetDataWindow(EditorUI& ui) : EditorWindow(ui)
{

}

void AssetDataWindow::draw()
{
	if(ImGui::Begin("Asset Data", nullptr, ImGuiWindowFlags_None)){
	}
	ImGui::End();
}
