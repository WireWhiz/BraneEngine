//
// Created by eli on 5/23/2022.
//

#ifndef BRANEENGINE_MEMORYMANAGERWINDOW_H
#define BRANEENGINE_MEMORYMANAGERWINDOW_H

#include "ui/guiWindow.h"
class EntityManager;
class MemoryManagerWindow : public GUIWindow
{
    EntityManager* _em;
public:
	MemoryManagerWindow(GUI& ui);
	void draw() override;
};


#endif //BRANEENGINE_MEMORYMANAGERWINDOW_H
