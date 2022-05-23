//
// Created by eli on 4/26/2022.
//

#include "editorUI.h"
#include "windows/LoginWindow.h"
#include "editor/ui/windows/AssetDataWindow.h"
#include "editor/ui/windows/ConsoleWindow.h"
#include "editor/ui/windows/EntitiesWindow.h"
#include "editor/ui/windows/RenderWindow.h"
#include "editor/ui/windows/AssetBrowserWindow.h"

#include <runtime/runtime.h>

EditorUI::EditorUI(Runtime& runtime) : Module(runtime)
{
	runtime.timeline().addBlockBefore("editorUI", "draw");
	runtime.timeline().addTask("drawEditorUI", [&]{
		drawUI();
	}, "editorUI");

	addEventListener("loginSuccessful", std::function([this](LoginEvent* evt){
		_server = evt->server();
		for(auto window = _windows.begin(); window != _windows.end(); window++)
		{
			if(dynamic_cast<LoginWindow*>(window->get()))
			{
				_windows.erase(window);
				break;
			}
		}
		addMainWindows();
		_redockQueued = true;
	}));

	_windows.push_back(std::make_unique<LoginWindow>(*this));
}

const char* EditorUI::name()
{
	return "editorUI";
}

void EditorUI::drawUI()
{
	mainMenu();

	ImGui::SetNextWindowPos(ImGui::GetCurrentContext()->CurrentViewport->WorkPos);
	ImGui::SetNextWindowSize(ImGui::GetCurrentContext()->CurrentViewport->WorkSize);
	ImGui::SetNextWindowViewport(ImGui::GetCurrentContext()->CurrentViewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0,0});
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	if(ImGui::Begin("RootWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::PopStyleVar(3);
		ImGui::DockSpace(ImGui::GetID("DockingRoot"),{0,0}, ImGuiDockNodeFlags_None);

		if(_redockQueued){
			defaultDocking();
			_redockQueued = false;
		}
	}
	ImGui::End();


	for(auto& w : _windows)
	{
		w->draw();
	}

	callEvents();
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
	ImGuiID root = ImGui::GetID("DockingRoot");
	ImGui::DockBuilderRemoveNode(root);
	root = ImGui::DockBuilderAddNode(root, ImGuiDockNodeFlags_DockSpace |  ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton);
	ImGui::DockBuilderSetNodeSize(root, ImGui::GetCurrentContext()->CurrentViewport->WorkSize);

	ImGuiID assetDataWindow = ImGui::DockBuilderSplitNode(root, ImGuiDir_Right, .2f, nullptr, &root);
	ImGuiID consoleWindow = ImGui::DockBuilderSplitNode(root, ImGuiDir_Down, .2f, nullptr, &root);
	ImGuiID entitiesWindow = ImGui::DockBuilderSplitNode(root, ImGuiDir_Left, .2f, nullptr, &root);

	ImGui::DockBuilderDockWindow("Asset Data", assetDataWindow);
	ImGui::DockBuilderDockWindow("Console", consoleWindow);
	ImGui::DockBuilderDockWindow("Asset Browser", consoleWindow);
	ImGui::DockBuilderDockWindow("Entities", entitiesWindow);
	ImGui::DockBuilderDockWindow("Render", root);

	ImGui::DockBuilderFinish(root);
}

void EditorUI::addMainWindows()
{
	_windows.push_back(std::make_unique<AssetDataWindow>(*this));
	_windows.push_back(std::make_unique<AssetBrowserWindow>(*this));
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

void EditorUI::sendEvent(std::unique_ptr<EditorEvent>&& event)
{
	_queueLock.lock();
	_queuedEvents.push_back(std::move(event));
	_queueLock.unlock();
}

void EditorUI::callEvents()
{
	//Avoid mutex locks by moving event queue into local queue
	_queueLock.lock();
	std::deque events = std::move(_queuedEvents);
	assert(_queuedEvents.empty());
	_queueLock.unlock();

	for(auto& event : events)
	{
		if(_eventListeners.count(event->name()))
		{
			for(auto& listener : _eventListeners.at(event->name()))
			{
				listener(event.get());
			}
		}
	}
}

net::Connection* EditorUI::server() const
{
	return _server;
}


