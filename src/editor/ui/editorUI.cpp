//
// Created by eli on 4/26/2022.
//

#include "editorUI.h"
#include "editor/ui/windows/LoginWindow.h"
#include <runtime/runtime.h>

EditorUI::EditorUI(Runtime& runtime) : Module(runtime)
{
	runtime.timeline().addBlockBefore("editorUI", "draw");
	runtime.timeline().addTask("drawEditorUI", [&]{
		drawUI();
	}, "editorUI");

	_windows.push_back(std::make_unique<LoginWindow>(*this));
}

const char* EditorUI::name()
{
	return "editorUI";
}

void EditorUI::drawUI()
{
	mainMenu();

	for(auto& w : _windows)
	{
		w->draw();
	}
}

void EditorUI::mainMenu()
{
	if(ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("File"))
		{
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Window"))
		{
			if(ImGui::MenuItem("Reset Docking"))
			{

			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
