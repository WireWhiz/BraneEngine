//
// Created by eli on 5/22/2022.
//

#ifndef BRANEENGINE_ASSETBROWSERWINDOW_H
#define BRANEENGINE_ASSETBROWSERWINDOW_H

#include "editorWindow.h"
#include <string>
#include "../widgets/assetBrowserWidget.h"

class AssetBrowserWindow : public EditorWindow
{
    AssetBrowserWidget _browser;
    void displayContent() override;
public:
    AssetBrowserWindow(GUI& ui, Editor& editor);
};


#endif //BRANEENGINE_ASSETBROWSERWINDOW_H
