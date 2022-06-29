//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_CONSOLEWINDOW_H
#define BRANEENGINE_CONSOLEWINDOW_H

#include <ui/guiWindow.h>

class ConsoleWindow : public GUIWindow
{
public:
	ConsoleWindow(GUI& ui, GUIWindowID id);
	void draw() override;
};


#endif //BRANEENGINE_CONSOLEWINDOW_H
