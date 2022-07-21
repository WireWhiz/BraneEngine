//
// Created by eli on 5/16/2022.
//

#include "guiWindow.h"
#include "runtime/runtime.h"
#include "gui.h"

GUIWindow::GUIWindow(GUI& ui, GUIWindowID id) : _ui(ui), _id(id)
{

}

void GUIWindow::update()
{
    if(!_open)
    {
        _ui.removeWindow(_id);
    }
}

GUIWindowID GUIWindow::id() const
{
	return _id;
}