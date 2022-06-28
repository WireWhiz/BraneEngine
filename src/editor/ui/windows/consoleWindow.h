//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_CONSOLEWINDOW_H
#define BRANEENGINE_CONSOLEWINDOW_H

#include "../editorWindow.h"

class ConsoleWindow : public EditorWindow
{
public:
	ConsoleWindow(EditorUI& ui);
	void draw() override;
};


#endif //BRANEENGINE_CONSOLEWINDOW_H
