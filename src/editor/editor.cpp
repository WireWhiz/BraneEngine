//
// Created by eli on 6/28/2022.
//

#include "editor.h"

#include "ui/gui.h"
#include "windows/loginWindow.h"
#include "windows/dataWindow.h"
#include "windows/consoleWindow.h"
#include "windows/entitiesWindow.h"
#include "windows/renderWindow.h"
#include "windows/assetBrowserWindow.h"
#include "editorEvents.h"
#include <fileManager/fileManager.h>
#include <assets/assetManager.h>
#include "windows/memoryManagerWindow.h"
#include "graphics/material.h"
#include "graphics/graphics.h"
#include "networking/networking.h"
#include "assetEditorContext.h"

void Editor::start()
{
	_ui = Runtime::getModule<GUI>();
	auto* loginWindow = _ui->addWindow<LoginWindow>();

	_ui->addEventListener("login", nullptr, std::function([this, loginWindow](const LoginEvent* evt){
		_server = evt->server();
        loginWindow->close();
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
    auto* dataWindow = _ui->addWindow<DataWindow>();
    auto* assetBrowser = _ui->addWindow<AssetBrowserWindow>();
    auto* console = _ui->addWindow<ConsoleWindow>();
    auto* entities = _ui->addWindow<EntitiesWindow>();
    auto* renderer = _ui->addWindow<RenderWindow>();

	ImGuiID root = ImGui::GetID("DockingRoot");
	ImGui::DockBuilderRemoveNode(root);
	root = ImGui::DockBuilderAddNode(root, ImGuiDockNodeFlags_DockSpace |  ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton);
	ImGui::DockBuilderSetNodeSize(root, ImGui::GetCurrentContext()->CurrentViewport->WorkSize);

	ImGuiID assetDataNode = ImGui::DockBuilderSplitNode(root, ImGuiDir_Right, .2f, nullptr, &root);
	ImGuiID consoleWindowNode = ImGui::DockBuilderSplitNode(root, ImGuiDir_Down, .2f, nullptr, &root);
	ImGuiID entitiesWindowNode = ImGui::DockBuilderSplitNode(root, ImGuiDir_Left, .2f, nullptr, &root);

	ImGui::DockBuilderDockWindow(dataWindow->name().c_str(), assetDataNode);
	ImGui::DockBuilderDockWindow(console->name().c_str(), consoleWindowNode);
	ImGui::DockBuilderDockWindow(assetBrowser->name().c_str(), consoleWindowNode);
	ImGui::DockBuilderDockWindow(entities->name().c_str(), entitiesWindowNode);
	ImGui::DockBuilderDockWindow(renderer->name().c_str(), root);

	//Attempt at recreating the (for some reason commented out) ImGuiDockNodeFlags_NoCentralNode
	ImGui::DockBuilderGetNode(root)->MergedFlags ^= ImGuiDockNodeFlags_CentralNode;
	ImGui::DockBuilderGetNode(root)->UpdateMergedFlags();

	ImGui::DockBuilderFinish(root);


	Runtime::log("Main layout loaded");
}

net::Connection* Editor::server() const
{
	return _server;
}

void Editor::drawMenu()
{
	if(ImGui::BeginMainMenuBar())
	{
		if(ImGui::BeginMenu("File"))
		{
            ImGui::Selectable("Create Asset");
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Window"))
		{
            if(ImGui::Selectable("Entities"))
                _ui->addWindow<EntitiesWindow>()->resizeDefault();
            if(ImGui::Selectable("Asset Browser"))
                _ui->addWindow<AssetBrowserWindow>()->resizeDefault();
            if(ImGui::Selectable("Render Preview"))
                _ui->addWindow<RenderWindow>()->resizeDefault();
            if(ImGui::Selectable("Console"))
                _ui->addWindow<ConsoleWindow>()->resizeDefault();
            if(ImGui::Selectable("Data Inspector"))
                _ui->addWindow<DataWindow>()->resizeDefault();
            if(ImGui::Selectable("Memory Manager"))
                _ui->addWindow<MemoryManagerWindow>()->resizeDefault();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

std::shared_ptr<AssetEditorContext> Editor::getEditorContext(const AssetID& id)
{
    if(_assetContexts.count(id))
        return _assetContexts.at(id);

    auto* am = Runtime::getModule<AssetManager>();
    Asset* asset = am->getAsset<Asset>(id);
    if(!asset)
    {
        Runtime::warn("Tried to create editor context but " + id.string() + " is not loaded!");
        return nullptr;
    }

    _assetContexts.insert({id, std::make_shared<AssetEditorContext>(asset)});
    return _assetContexts.at(id);
}
