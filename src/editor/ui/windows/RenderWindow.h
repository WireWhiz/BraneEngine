//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_RENDERWINDOW_H
#define BRANEENGINE_RENDERWINDOW_H

#include "../editorWindow.h"

class RenderWindow : public EditorWindow
{
public:
	RenderWindow(EditorUI& ui);
	void draw() override;
};


#endif //BRANEENGINE_RENDERWINDOW_H
