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

	auto& nm = *Runtime::getModule<NetworkManager>();
	auto& am = *Runtime::getModule<AssetManager>();
	am.setFetchCallback([&](auto id, auto incremental){
		AsyncData<Asset*> asset;
		nm.async_connectToAssetServer(id.serverAddress, Config::json()["network"]["tcp_port"].asUInt(), [&nm, id, incremental, asset](bool connected){
			if(!connected)
			{
				asset.setError("Could not connect to server: " + id.serverAddress);
				std::cerr << "Could not get asset: " << id << std::endl;
				return;
			}
			if (incremental)
			{
				nm.async_requestAssetIncremental(id).then([asset, id](Asset* data){
					asset.setData(data);
				});
			}
			else
			{
				AsyncData<Asset*> assetToSave;
				nm.async_requestAsset(id).then([asset, id](Asset* data){
					asset.setData(data);
				});
			}
		});
		return asset;
	});
    auto* vkr = Runtime::getModule<graphics::VulkanRuntime>();
    auto* em = Runtime::getModule<EntityManager>();
   _defaultMaterial = new graphics::Material();
    _defaultMaterial->setVertex(vkr->loadShader(0));
    _defaultMaterial->setFragment(vkr->loadShader(1));
    //mat->addTextureDescriptor(vkr.loadTexture(0));
    _defaultMaterial->addBinding(0,sizeof(glm::vec3));
    _defaultMaterial->addBinding(1, sizeof(glm::vec3));
    _defaultMaterial->addAttribute(0, VK_FORMAT_R32G32B32_SFLOAT, 0);
    _defaultMaterial->addAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, 0);
    vkr->addMaterial(_defaultMaterial);
    em->components().registerComponent(_defaultMaterial->component());

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

	ImGui::DockBuilderDockWindow("Data Inspector", assetDataWindow);
	ImGui::DockBuilderDockWindow("Console", consoleWindow);
	ImGui::DockBuilderDockWindow("Asset Browser", consoleWindow);
	ImGui::DockBuilderDockWindow("Entities", entitiesWindow);
	ImGui::DockBuilderDockWindow("Render", root);

	//Attempt at recreating the (for some reason commented out) ImGuiDockNodeFlags_NoCentralNode
	ImGui::DockBuilderGetNode(root)->MergedFlags ^= ImGuiDockNodeFlags_CentralNode;
	ImGui::DockBuilderGetNode(root)->UpdateMergedFlags();

	ImGui::DockBuilderFinish(root);

	_ui->addWindow<DataWindow>();
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
                _ui->addWindow<EntitiesWindow>();
            if(ImGui::Selectable("Asset Browser"))
                _ui->addWindow<AssetBrowserWindow>();
            if(ImGui::Selectable("Render Preview"))
                _ui->addWindow<RenderWindow>();
            if(ImGui::Selectable("Console"))
                _ui->addWindow<ConsoleWindow>();
            if(ImGui::Selectable("Data Inspector"))
                _ui->addWindow<DataWindow>();
            if(ImGui::Selectable("Memory Manager"))
                _ui->addWindow<MemoryManagerWindow>();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

graphics::Material* Editor::defaultMaterial() const
{
    return _defaultMaterial;
}
