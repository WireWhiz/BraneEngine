//
// Created by eli on 5/23/2022.
//

#ifndef BRANEENGINE_MEMORYMANAGERWINDOW_H
#define BRANEENGINE_MEMORYMANAGERWINDOW_H

#include "../editorWindow.h"
class MemoryManagerWindow : public EditorWindow
{
public:
	MemoryManagerWindow(EditorUI& ui);
	void draw() override;
};


#endif //BRANEENGINE_MEMORYMANAGERWINDOW_H
