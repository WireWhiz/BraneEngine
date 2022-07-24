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



void AssetBrowserWindow::importAsset()
{
	/*assert(_currentDir);
	net::Request req("processAsset");
	auto lastSlash = std::max(_filePath.find_last_of('/'), _filePath.find_last_of('\\'));
	if(lastSlash == std::string::npos)
		lastSlash = 0;
	std::string assetFilename = _filePath.substr(lastSlash);
	std::string assetPath = _currentDir->path();

	std::string assetData;
	FileManager::readFile(_filePath, assetData);

	req.body() << assetFilename << _name << assetPath << assetData;

	Directory* dir = _currentDir;
	Runtime::getModule<Editor>()->server()->sendRequest(req).then([this, dir](ISerializedData sData)
    {
        bool success;
        sData >> success;
        if (!success)
        {
			Runtime::error("Could not import asset");
			return;
		}
	    fetchDirectory(dir);
    });*/
}


