//
// Created by eli on 4/26/2022.
//

#ifndef BRANEENGINE_EDITORUI_H
#define BRANEENGINE_EDITORUI_H

#include <imgui.h>
#include <imgui_internal.h>
#include <runtime/module.h>

class EditorUI : public Module
{


	void drawUI();
public:
	EditorUI(Runtime& runtime);

	const char* name() override;

};


#endif //BRANEENGINE_EDITORUI_H
