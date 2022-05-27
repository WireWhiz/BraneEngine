//
// Created by eli on 5/16/2022.
//

#ifndef BRANEENGINE_EDITORWINDOW_H
#define BRANEENGINE_EDITORWINDOW_H

#include <imgui.h>
#include <imgui_internal.h>
#include "IconsFontAwesome6.h"

class EditorUI;
class EditorWindow
{
protected:
	EditorUI& _ui;
public:
	EditorWindow(EditorUI& ui);
	virtual ~EditorWindow() = default;
	virtual void draw() = 0;
	virtual void update();
};


#endif //BRANEENGINE_EDITORWINDOW_H
