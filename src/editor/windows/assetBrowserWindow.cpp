//
// Created by eli on 5/22/2022.
//

#include "assetBrowserWindow.h"
#include <ui/gui.h>
#include <stack>
#include "common/utility/hex.h"
#include "../editor.h"
#include "misc/cpp/imgui_stdlib.h"
#include "editor/editorEvents.h"
#include <algorithm>
#include <utility/strCaseCompare.h>
#include <fileManager/fileManager.h>
#define CLIENT
#include <assets/assetManager.h>

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


