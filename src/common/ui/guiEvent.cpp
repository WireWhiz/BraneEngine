//
// Created by eli on 5/22/2022.
//

#include "guiEvent.h"

GUIEvent::GUIEvent(std::string name)
{
	_name = name;
}

const std::string& GUIEvent::name() const
{
	return _name;
}
