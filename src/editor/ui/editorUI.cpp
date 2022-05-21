//
// Created by eli on 4/26/2022.
//

#include "editorUI.h"
#include "windows/LoginWindow.h"
#include "editor/ui/windows/AssetDataWindow.h"
#include "editor/ui/windows/ConsoleWindow.h"
#include "editor/ui/windows/EntitiesWindow.h"
#include "editor/ui/windows/RenderWindow.h"

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
	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

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

Runtime& EditorUI::runtime()
{
	return _rt;
}

void EditorUI::defaultDocking()
{
	ImGuiID root = ImGui::DockBuilderAddNode();
	ImGui::DockBuilderSetNodePos(root, ImGui::GetMainViewport()->WorkPos);
	ImGui::DockBuilderSetNodeSize(root, ImGui::GetMainViewport()->WorkSize);

	ImGuiID assetDataWindow = ImGui::DockBuilderSplitNode(root, ImGuiDir_Right, .2f, NULL, &root);
	ImGuiID consoleWindow = ImGui::DockBuilderSplitNode(root, ImGuiDir_Down, .2f, NULL, &root);
	ImGuiID entitiesWindow = ImGui::DockBuilderSplitNode(root, ImGuiDir_Left, .2f, NULL, &root);

	ImGui::DockBuilderDockWindow("Asset Data", assetDataWindow);
	ImGui::DockBuilderDockWindow("Console", consoleWindow);
	ImGui::DockBuilderDockWindow("Entities", entitiesWindow);
	ImGui::DockBuilderDockWindow("Render", root);
	ImGui::DockBuilderFinish(root);
}

void EditorUI::addMainWindows()
{
	_windows.push_back(std::make_unique<AssetDataWindow>(*this));
	_windows.push_back(std::make_unique<ConsoleWindow>(*this));
	_windows.push_back(std::make_unique<EntitiesWindow>(*this));
	_windows.push_back(std::make_unique<RenderWindow>(*this));
}

void EditorUI::removeWindow(EditorWindow* window)
{
	for(auto i = _windows.begin(); i != _windows.end(); i++)
	{
		if(i->get() == window)
		{
			_windows.erase(i);
			return;
		}
	}
}


