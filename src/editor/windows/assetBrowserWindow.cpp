//
// Created by eli on 5/22/2022.
//

#include "assetBrowserWindow.h"
#include <ui/gui.h>

AssetBrowserWindow::AssetBrowserWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id), _browser(ui, true)
{

}

void AssetBrowserWindow::draw()
{
	if(ImGui::Begin("Asset Browser", &_open, ImGuiWindowFlags_NoScrollbar))
	{
        _browser.displayFullBrowser();
	}
	ImGui::End();
}


