//
// Created by eli on 6/28/2022.
//

#include "editor.h"

#include "ui/gui.h"
#include "windows/dataWindow.h"
#include "windows/consoleWindow.h"
#include "windows/entitiesWindow.h"
#include "windows/renderWindow.h"
#include "windows/assetBrowserWindow.h"
#include "windows/syncWindow.h"
#include "windows/selectProjectWindow.h"
#include "windows/memoryManagerWindow.h"

#include "editorEvents.h"
#include <fileManager/fileManager.h>
#include <assets/assetManager.h>
#include "graphics/material.h"
#include "graphics/graphics.h"
#include "networking/networking.h"
#include "assets/editorAsset.h"

void Editor::start()
{
	_ui = Runtime::getModule<GUI>();
	_selectProjectWindow = _ui->addWindow<SelectProjectWindow>();

	_ui->setMainMenuCallback([this](){drawMenu();});
	_ui->addEventListener<GUIEvent>("projectLoaded", nullptr, [this](auto evt){
		if(_selectProjectWindow)
		{
			_selectProjectWindow = nullptr;
			_ui->clearWindows();
			addMainWindows();
		}
	});
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
			if(ImGui::Selectable("Sync Window"))
				_ui->addWindow<SyncWindow>()->resizeDefault();
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

BraneProject& Editor::project()
{
	return _project;
}

void Editor::loadProject(const std::filesystem::path& filepath)
{
	_project.load(filepath);
	_ui->sendEvent(std::make_unique<GUIEvent>("projectLoaded"));
}

void Editor::createProject(const std::string& name, const std::filesystem::path& directory)
{
	_project.create(name, directory);
	_ui->sendEvent(std::make_unique<GUIEvent>("projectLoaded"));
}

JsonVersionTracker& Editor::jsonTracker()
{
	return _jsonTracker;
}

Editor::Editor() : _project(_jsonTracker)
{

}

//The editor specific fetch asset function
AsyncData<Asset*> AssetManager::fetchAsset(const AssetID& id, bool incremental)
{
	AsyncData<Asset*> asset;
	if(hasAsset(id))
	{
		asset.setData(getAsset<Asset>(id));
		return asset;
	}

	AssetData* assetData = new AssetData{};
	assetData->loadState = LoadState::requested;
	_assetLock.lock();
	_assets.insert({id, std::unique_ptr<AssetData>(assetData)});
	_assetLock.unlock();

	auto* nm = Runtime::getModule<NetworkManager>();
	if(incremental)
	{
		nm->async_requestAssetIncremental(id).then([this, asset](Asset* ptr){
			auto& d = _assets.at(ptr->id);
			d->loadState = LoadState::usable;
			d->asset = std::unique_ptr<Asset>(ptr);
			ptr->onDependenciesLoaded();
			asset.setData(ptr);
		});
	}
	else
	{
		nm->async_requestAsset(id).then([this, asset](Asset* ptr){
			AssetID id = ptr->id;
			auto& d = _assets.at(id);
			d->loadState = LoadState::awaitingDependencies;
			d->asset = std::unique_ptr<Asset>(ptr);
			if(dependenciesLoaded(d->asset.get()))
			{
				d->loadState = LoadState::loaded;
				d->asset->onDependenciesLoaded();
				asset.setData(ptr);
				return;
			}
			d->loadState = LoadState::awaitingDependencies;
			fetchDependencies(d->asset.get(), [this, id, asset]() mutable{
				auto& d = _assets.at(id);
				d->loadState = LoadState::loaded;
				d->asset->onDependenciesLoaded();
				asset.setData(d->asset.get());
			});
		});
	}
	return asset;
}