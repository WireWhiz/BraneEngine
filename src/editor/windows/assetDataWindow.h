//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_ASSETDATAWINDOW_H
#define BRANEENGINE_ASSETDATAWINDOW_H

#include <ui/guiWindow.h>

class AssetDataWindow : public GUIWindow
{
public:
	AssetDataWindow(GUI& ui, GUIWindowID id);
	void draw() override;
};


#endif //BRANEENGINE_ASSETDATAWINDOW_H
