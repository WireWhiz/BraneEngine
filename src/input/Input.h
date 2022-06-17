//
// Created by eli on 6/2/2022.
//

#ifndef BRANEENGINE_INPUT_H
#define BRANEENGINE_INPUT_H

#include "runtime/module.h"
#include "graphics/window.h"

class InputManager : public Module
{
	graphics::Window* _window;
public:
	InputManager();
	void start() override;
};


#endif //BRANEENGINE_INPUT_H
