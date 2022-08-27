//
// Created by eli on 5/22/2022.
//

#include "assetBrowserWindow.h"
#include <ui/gui.h>

AssetBrowserWindow::AssetBrowserWindow(GUI& ui, Editor& editor) : EditorWindow(ui, editor), _browser(ui, true)
{
    _flags = ImGuiWindowFlags_NoScrollbar;
    _name = "Asset Browser";
}

void AssetBrowserWindow::displayContent()
{
    _browser.displayFullBrowser();
}


