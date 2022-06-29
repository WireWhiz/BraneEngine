//
// Created by eli on 5/16/2022.
//

#include "guiWindow.h"

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