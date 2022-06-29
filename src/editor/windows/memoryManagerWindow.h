//
// Created by eli on 5/23/2022.
//

#ifndef BRANEENGINE_MEMORYMANAGERWINDOW_H
#define BRANEENGINE_MEMORYMANAGERWINDOW_H

#include "ui/guiWindow.h"
class MemoryManagerWindow : public GUIWindow
{
public:
	MemoryManagerWindow(GUI& ui, GUIWindowID id);
	void draw() override;
};


#endif //BRANEENGINE_MEMORYMANAGERWINDOW_H
