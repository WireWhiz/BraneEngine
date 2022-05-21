//
// Created by eli on 4/26/2022.
//

#ifndef BRANEENGINE_EDITORUI_H
#define BRANEENGINE_EDITORUI_H

#include <runtime/module.h>
#include "editorWindow.h"
#include <memory>
#include <vector>
#include <atomic>


class EditorUI : public Module
{
	std::vector<std::unique_ptr<EditorWindow>> _windows;
	void drawUI();
	void mainMenu();
public:
	EditorUI(Runtime& runtime);

	const char* name() override;
	Runtime& runtime();

	void removeWindow(EditorWindow* window);

	void defaultDocking();
	void addMainWindows();
};


#endif //BRANEENGINE_EDITORUI_H
