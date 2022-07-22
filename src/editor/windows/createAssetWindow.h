//
// Created by wirewhiz on 20/07/22.
//

#ifndef BRANEENGINE_CREATEASSETWINDOW_H
#define BRANEENGINE_CREATEASSETWINDOW_H

#include <ui/guiWindow.h>

class CreateAssetWindow : public GUIWindow
{
    void loadAssetFromFile(const std::string &filename);
public:
    CreateAssetWindow(GUI& ui, GUIWindowID id);
    void draw() override;
};


#endif //BRANEENGINE_CREATEASSETWINDOW_H
