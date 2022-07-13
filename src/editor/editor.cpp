//
// Created by eli on 6/28/2022.
//

#include "editor.h"

#include "windows/loginWindow.h"
#include "windows/assetDataWindow.h"
#include "windows/consoleWindow.h"
#include "windows/entitiesWindow.h"
#include "windows/renderWindow.h"
#include "windows/assetBrowserWindow.h"
#include <fileManager/fileManager.h>


void Editor::start()
{
	Module::start();
	_ui = Runtime::getModule<GUI>();
	GUIWindowID loginWindow = _ui->addWindow<LoginWindow>()->id();

	_ui->addEventListener("loginSuccessful", std::function([this, loginWindow](LoginEvent* evt){
		_server = evt->server();
		_ui->removeWindow(loginWindow);
		addMainWindows();
		_ui->setMainMenuCallback([this](){drawMenu();});
	}));


}

const char* Editor::name()
{
	return "editor";
}

void Editor::addMainWindows()
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

	//Attempt at recreating the (for some reason commented out) ImGuiDockNodeFlags_NoCentralNode
	ImGui::DockBuilderGetNode(root)->MergedFlags ^= ImGuiDockNodeFlags_CentralNode;
	ImGui::DockBuilderGetNode(root)->UpdateMergedFlags();

	ImGui::DockBuilderFinish(root);

	_ui->addWindow<AssetDataWindow>();
	_ui->addWindow<AssetBrowserWindow>();
	_ui->addWindow<ConsoleWindow>();
	_ui->addWindow<EntitiesWindow>();
	_ui->addWindow<RenderWindow>();
	Runtime::log("Main layout loaded");
}

net::Connection* Editor::server() const
{
	return _server;
}

void Editor::importAsset()
{
	std::string path = FileManager::requestLocalFilePath("Import Asset", {"*.gltf","*.glb","*.png"});
	Runtime::log("Attempting to import: " + path);
}

void Editor::drawMenu()
{
	if(ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("File"))
		{

			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Window"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
