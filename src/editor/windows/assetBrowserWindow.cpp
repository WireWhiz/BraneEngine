//
// Created by eli on 5/22/2022.
//

#include "assetBrowserWindow.h"
#include <ui/gui.h>
#include <stack>
#include "common/utility/hex.h"
#include "../editor.h"

AssetBrowserWindow::AssetBrowserWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{
	_root.name = "root";
	setDirectory(&_root);
}

void AssetBrowserWindow::draw()
{
	if(ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::TextDisabled("%s",_strPath.c_str());
		ImGui::Separator();

		if(ImGui::BeginTable("window split", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInnerV, ImGui::GetContentRegionAvail()))
		{
			ImGui::TableNextColumn();
			displayDirectories();
			ImGui::TableNextColumn();
			if(!_currentDir)
				ImGui::Text("No directory selected");
			else if(!_currentDir->loaded)
				ImGui::Text("Loading...");
			else if(_currentDir->children.empty() && _currentDir->files.empty())
				ImGui::Text("No Assets");
			else
			{
				ImGui::BeginChild("Directory Contents", {0,0}, false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, {0.0f, 0.6f});
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.4f,0.4f,0.5f, 1});
				ImGui::PushStyleColor(ImGuiCol_Button, {0.1f,0.1f,0.1f, 1});
				for(auto& dir : _currentDir->children)
					if(ImGui::ButtonEx(std::string(ICON_FA_FOLDER " " + dir->name).c_str(), {ImGui::GetContentRegionAvail().x,0}))
					{
						setDirectory(dir.get());
					}
				for(auto& file : _currentDir->files)
					if(ImGui::ButtonEx(std::string(ICON_FA_FILE " " + file).c_str(),{ImGui::GetContentRegionAvail().x,0}))
					{
						//TODO: Be able to click files
					}
				ImGui::PopStyleVar(1);
				ImGui::PopStyleColor(2);
				ImGui::EndChild();
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void AssetBrowserWindow::displayDirectories()
{
	displayDirectoriesRecursive(&_root);
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
	if(!dir->loaded)
	{
		net::Request req("directoryContents");
		req.body() << _strPath;
		Runtime::getModule<Editor>()->server()->sendRequest(req).then([this, dir](ISerializedData sData){
			std::vector<std::string> directories;
			bool success;
			sData >> success >> directories >> dir->files;
			if(!success)
				return; //TODO show could not load error here instead
			for(auto& d : directories)
			{
				auto newDir = std::make_unique<Directory>();
				newDir->name = d;
				newDir->parent = dir;
				dir->children.push_back(std::move(newDir));
			}

			dir->loaded = true;
		});
	}
}

void AssetBrowserWindow::updateStrPath()
{
	_strPath = "/";
	std::stack<Directory*> dirNames;
	for(Directory* dir = _currentDir; dir != &_root; dir = dir->parent)
		dirNames.push(dir);
	while(!dirNames.empty())
	{
		_strPath += dirNames.top()->name + "/";
		dirNames.pop();
	}
}


