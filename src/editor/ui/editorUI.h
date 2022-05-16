//
// Created by eli on 4/26/2022.
//

#ifndef BRANEENGINE_EDITORUI_H
#define BRANEENGINE_EDITORUI_H

#include <runtime/module.h>
#include "editorWindow.h"
#include <memory>
#include <vector>


class EditorUI : public Module
{

	std::vector<std::unique_ptr<EditorWindow>> _windows;
	void drawUI();
	void mainMenu();
public:
	EditorUI(Runtime& runtime);

	const char* name() override;

};


#endif //BRANEENGINE_EDITORUI_H
