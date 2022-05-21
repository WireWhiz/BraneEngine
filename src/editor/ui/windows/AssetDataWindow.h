//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_ASSETDATAWINDOW_H
#define BRANEENGINE_ASSETDATAWINDOW_H

#include "../editorWindow.h"

class AssetDataWindow : public EditorWindow
{
public:
	AssetDataWindow(EditorUI& ui);
	void draw() override;
};


#endif //BRANEENGINE_ASSETDATAWINDOW_H
