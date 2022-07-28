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

}

GUIWindowID GUIWindow::id() const
{
	return _id;
}

bool GUIWindow::isOpen() const
{
    return _open;
}

void GUIWindow::close()
{
    _open = false;
}
