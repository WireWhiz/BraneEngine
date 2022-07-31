//
// Created by eli on 5/16/2022.
//

#include "guiWindow.h"
#include "runtime/runtime.h"
#include "gui.h"

GUIWindow::GUIWindow(GUI& ui) : _ui(ui)
{

}

void GUIWindow::update()
{

}

bool GUIWindow::isOpen() const
{
    return _open;
}

void GUIWindow::close()
{
    _open = false;
}
