//
// Created by eli on 5/22/2022.
//

#ifndef BRANEENGINE_ASSETBROWSERWINDOW_H
#define BRANEENGINE_ASSETBROWSERWINDOW_H


#include <ui/guiWindow.h>
#include "networking/networking.h"
#include <string>
#include "../widgets/assetBrowserWidget.h"

class AssetBrowserWindow : public GUIWindow
{
	AssetBrowserWidget _browser;

	std::string _name;
	std::string _filePath;

	void importAsset();

public:
	AssetBrowserWindow(GUI& ui, GUIWindowID id);
	void draw() override;
};


#endif //BRANEENGINE_ASSETBROWSERWINDOW_H
