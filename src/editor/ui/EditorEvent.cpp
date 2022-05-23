//
// Created by eli on 5/22/2022.
//

#include "EditorEvent.h"

EditorEvent::EditorEvent(std::string name)
{
	_name = name;
}

const std::string& EditorEvent::name() const
{
	return _name;
}
