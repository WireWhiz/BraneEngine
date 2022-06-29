//
// Created by eli on 5/21/2022.
//

#include "assetDataWindow.h"

AssetDataWindow::AssetDataWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{

}

void AssetDataWindow::draw()
{
	if(ImGui::Begin("Asset Data", nullptr, ImGuiWindowFlags_None)){
	}
	ImGui::End();
}
