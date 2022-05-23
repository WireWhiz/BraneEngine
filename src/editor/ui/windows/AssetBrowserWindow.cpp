//
// Created by eli on 5/22/2022.
//

#include "AssetBrowserWindow.h"
#include "../editorUI.h"
#include <stack>
#include <utility/hex.h>
#include <graphics/IconsFontAwesome6.h>

AssetBrowserWindow::AssetBrowserWindow(EditorUI& ui) : EditorWindow(ui),
                                                       _nm(*(NetworkManager*)(ui.runtime().getModule("networkManager")))
{
	ui.server()->sendRequest(net::Request("directoryTree")).then([this](ISerializedData sData){
		_root = deserializeDirectory(sData);
		setDirectory(_root.get());
	});
}

void AssetBrowserWindow::draw()
{
	if(ImGui::Begin("Asset Browser"))
	{
		ImGui::TextDisabled("%s",_strPath.c_str());
		ImGui::Separator();

		if(_root && ImGui::BeginTable("window split", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY, ImGui::GetContentRegionAvail()))
		{
			ImGui::TableNextColumn();
			displayDirectories();
			ImGui::TableNextColumn();
			if(!_currentDir)
				ImGui::Text("No directory selected");
			else if(_directoryContents.empty() && _currentDir->children.empty())
				ImGui::Text("No Assets");
			else
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, {0.0f, 0.6f});
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.4f,0.4f,0.5f, 1});
				ImGui::PushStyleColor(ImGuiCol_Button, {0.1f,0.1f,0.1f, 1});
				for(auto& dir : _currentDir->children)
					if(ImGui::ButtonEx(std::string(ICON_FA_FOLDER " " + dir->name).c_str(), {ImGui::GetContentRegionAvail().x,0}))
					{
						setDirectory(dir.get());
					}
				for(auto& asset : _directoryContents)
					if(ImGui::ButtonEx(std::string(ICON_FA_FILE " " + asset.name).c_str(),{ImGui::GetContentRegionAvail().x,0}))
					{

					}
				ImGui::PopStyleVar(1);
				ImGui::PopStyleColor(2);
			}

			ImGui::EndTable();
		}
		else
			ImGui::Text("Loading directory tree...");
	}
	ImGui::End();
}

void AssetBrowserWindow::displayDirectories()
{
	displayDirectoriesRecursive(_root.get());
}

void AssetBrowserWindow::displayDirectoriesRecursive(AssetBrowserWindow::Directory* dir)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	if(dir->children.empty())
		flags |= ImGuiTreeNodeFlags_Leaf;
	if(ImGui::TreeNodeEx(dir->name.c_str(), flags))
	{
		if(ImGui::IsItemClicked())
		{
			setDirectory(dir);
		}
		for(auto& dirChild : dir->children)
		{
			displayDirectoriesRecursive(dirChild.get());
		}
		ImGui::TreePop();
	}
	else if(ImGui::IsItemClicked())
	{
		_currentDir = dir;
		updateStrPath();
	}
}

void AssetBrowserWindow::setDirectory(Directory* dir)
{
	if(dir == _currentDir)
		return;
	_currentDir = dir;
	updateStrPath();
	net::Request req("directoryAssets");
	req.body() << _currentDir->id;
	_ui.server()->sendRequest(req).then([this, dir](ISerializedData sData){
		std::vector<AssetData> assets;
		uint32_t count;
		sData >> count;
		for (uint32_t i = 0; i < count; ++i)
		{
			AssetData asset;
			sData >> asset.id >> asset.name >> asset.type >> asset.directoryID;
			asset.hexIDString = toHex((uint64_t)asset.id);
			assets.push_back(std::move(asset));
		}
		_directoryContents = std::move(assets);
	});
}

void AssetBrowserWindow::updateStrPath()
{
	_strPath = "/";
	std::stack<Directory*> dirNames;
	for(Directory* dir = _currentDir; dir != nullptr; dir = dir->parent)
		dirNames.push(dir);
	while(!dirNames.empty())
	{
		_strPath += dirNames.top()->name + "/";
		dirNames.pop();
	}
}

std::unique_ptr<AssetBrowserWindow::Directory> AssetBrowserWindow::deserializeDirectory(ISerializedData& sData, Directory* parent)
{
	std::unique_ptr<AssetBrowserWindow::Directory> dir = std::make_unique<Directory>();
	sData >> dir->id >> dir->name;
	uint16_t children;
	sData >> children;
	for (uint16_t i = 0; i < children; ++i)
		dir->children.push_back(deserializeDirectory(sData, dir.get()));
	dir->parent = parent;
	return std::move(dir);
}


