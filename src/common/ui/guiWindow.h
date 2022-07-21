//
// Created by eli on 5/16/2022.
//

#ifndef BRANEENGINE_GUIWINDOW_H
#define BRANEENGINE_GUIWINDOW_H

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "IconsFontAwesome6.h"

typedef size_t GUIWindowID;

class GUI;
class GUIWindow
{
	GUIWindowID _id;
protected:
	GUI& _ui;
    bool _open = true;
public:
	GUIWindow(GUI& ui, GUIWindowID id);
	virtual ~GUIWindow() = default;
	virtual void draw() = 0;
	virtual void update();
	GUIWindowID id() const;
};


#endif //BRANEENGINE_GUIWINDOW_H
