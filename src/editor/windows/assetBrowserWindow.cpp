//
// Created by eli on 5/22/2022.
//

#include "assetBrowserWindow.h"
#include <ui/gui.h>
#include <stack>
#include "common/utility/hex.h"
#include "../editor.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include <utility/strCaseCompare.h>
#include <fileManager/fileManager.h>

AssetBrowserWindow::AssetBrowserWindow(GUI& ui, GUIWindowID id) : GUIWindow(ui, id)
{
	_root.name = "root";
	setDirectory(&_root);
}

void AssetBrowserWindow::draw()
{
	std::scoped_lock l(_directoryLock);
	if(ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::TextDisabled("/%s",_strPath.c_str());

		if(_currentDir != &_root)
		{
			ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - 10);
			if(ImGui::Selectable(ICON_FA_ARROW_LEFT))
				setDirectory(_currentDir->parent);
		};

		ImGui::Separator();

		if(ImGui::BeginTable("window split", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInnerV, ImGui::GetContentRegionAvail()))
		{
			ImGui::TableNextColumn();
			ImGui::BeginChild("Directory Tree", {0,0}, false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			displayDirectories();
			ImGui::EndChild();
			ImGui::TableNextColumn();
			ImGui::BeginChild("Directory Contents", {0,0}, false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			displayFiles();
			ImGui::EndChild();
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
	ImGui::SetNextItemOpen(dir->open, ImGuiCond_Always);
	bool nodeOpen = ImGui::TreeNodeEx((((dir->open && !dir->children.empty()) ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER) + dir->name).c_str(), flags);
	dir->open = nodeOpen;

	if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		setDirectory(dir);
	}
	if(dir != &_root && ImGui::BeginDragDropSource())
	{
		std::string path = dir->path();
		ImGui::SetDragDropPayload("directory", &dir, sizeof(Directory*));
		ImGui::Text("%s",dir->name.c_str());
		ImGui::EndDragDropSource();
	}

	if(ImGui::BeginDragDropTarget())
	{
		if(const ImGuiPayload* p = ImGui::AcceptDragDropPayload("directory"))
		{
			moveDirectory(*(Directory**)p->Data, dir);
		}
	}
	if(nodeOpen)
	{

		for(auto& dirChild : dir->children)
		{
			displayDirectoriesRecursive(dirChild.get());
		}
		ImGui::TreePop();
	}
}

void AssetBrowserWindow::setDirectory(Directory* dir)
{
	if(dir == _currentDir)
		return;
	_currentDir = dir;
	_currentDir->setParentsOpen();
	updateStrPath();
	if(!dir->loaded)
	{
		fetchDirectory(dir);
	}
}

void AssetBrowserWindow::updateStrPath()
{
	_strPath = _currentDir->path();
}

void AssetBrowserWindow::createDirectory()
{
	net::Request req("createDirectory");
	std::string dirPath = _strPath + "/" + _name;
	req.body() << dirPath;
	Runtime::getModule<Editor>()->server()->sendRequest(req).then([this](ISerializedData sData){
		bool success;
		sData >> success;
		if(!success)
		{
			Runtime::error("Could not create directory");
			return;
		}
		reloadCurrentDirectory();
	});
}

void AssetBrowserWindow::reloadCurrentDirectory()
{
	fetchDirectory(_currentDir);
}

void AssetBrowserWindow::deleteFile(const std::string& path)
{
	net::Request req("deleteFile");
	req.body() << path;
	Runtime::getModule<Editor>()->server()->sendRequest(req).then([this, path](ISerializedData sData){
		bool success;
		sData >> success;
		if(!success)
		{
			Runtime::error("Could not delete file: " + path);
			return;
		}

		if(path == _currentDir->path())
			setDirectory(_currentDir->parent);
		reloadCurrentDirectory();
	});
}

void AssetBrowserWindow::displayFiles()
{
	bool fileHovered = false;

	if (!_currentDir)
		ImGui::Text("No directory selected");
	else if (!_currentDir->loaded)
		ImGui::Text("Loading...");
	else if (_currentDir->children.empty() && _currentDir->files.empty())
		ImGui::Text("No Assets");
	else
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, {0.0f, 0.6f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.4f, 0.4f, 0.5f, 1});
		ImGui::PushStyleColor(ImGuiCol_Button, {0.1f, 0.1f, 0.1f, 1});
		for (auto& dir: _currentDir->children)
		{
			ImGui::PushID(dir->name.c_str());
			if (ImGui::ButtonEx(std::string(ICON_FA_FOLDER " " + dir->name).c_str(),
			                    {ImGui::GetContentRegionAvail().x, 0}))
			{
				setDirectory(dir.get());
			}
			fileHovered |= ImGui::IsItemHovered();

			if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("directoryActions");
			}

			if (ImGui::BeginPopup("directoryActions"))
			{
				if (ImGui::Selectable("Delete"))
				{
					//Todo: confirmation dialog
					deleteFile(dir->path());
				}
				ImGui::EndPopup();
			}

			if (ImGui::BeginDragDropSource())
			{
				std::string path = dir->path();
				auto* dirPtr = dir.get();
				ImGui::SetDragDropPayload("directory", &dirPtr, sizeof(Directory*));
				ImGui::Text("%s",dir->name.c_str());
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if(const ImGuiPayload* p = ImGui::AcceptDragDropPayload("directory"))
				{
					moveDirectory(*(Directory**)p->Data, dir.get());
				}
			}
			ImGui::PopID();
		}
		for (auto& file: _currentDir->files)
		{
			if (ImGui::ButtonEx(std::string(ICON_FA_FILE " " + file).c_str(), {ImGui::GetContentRegionAvail().x, 0}))
			{
				//TODO: Be able to click files
			}
			fileHovered |= ImGui::IsItemHovered();
		}

		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(2);
	}
	assert(_currentDir);

	ImGuiID newDirectoryPopup = ImGui::GetID("newDirectory");
	if (ImGui::BeginPopupEx(newDirectoryPopup, ImGuiPopupFlags_None))
	{
		ImGui::Text("Create Directory:");
		ImGui::InputText("##name", &_name);
		if (ImGui::Button("create"))
		{
			ImGui::CloseCurrentPopup();
			createDirectory();
		}
		ImGui::EndPopup();
	}

	ImGuiID importAssetPopup = ImGui::GetID("importAsset");
	if(ImGui::BeginPopupEx(importAssetPopup, ImGuiPopupFlags_None))
	{
		ImGui::Text("Create Asset");
		ImGui::Text("Source file: %s", _filePath.c_str());
		ImGui::InputText("name", &_name);
		if (ImGui::Button("create"))
		{
			ImGui::CloseCurrentPopup();
			importAsset();
		}
		ImGui::EndPopup();
	}

	if (!fileHovered && ImGui::BeginPopupContextWindow("directoryActions"))
	{

		if (ImGui::Selectable(ICON_FA_FOLDER " New Directory"))
		{
			_name = "new directory";
			ImGui::OpenPopup(newDirectoryPopup);
		}
		if (ImGui::Selectable(ICON_FA_FILE_IMPORT " Import Asset"))
		{
			_filePath = FileManager::requestLocalFilePath("Import Asset", {"*.glb","*.png"});
			_name = "new asset";
			if(!_filePath.empty())
				ImGui::OpenPopup(importAssetPopup);
		}

		ImGui::EndPopup();
	}
}

void AssetBrowserWindow::fetchDirectory(AssetBrowserWindow::Directory* dir)
{
	net::Request req("directoryContents");
	req.body() << dir->path();

	Runtime::getModule<Editor>()->server()->sendRequest(req).then([this, dir](ISerializedData sData){

		bool success;
		sData >> success;
		if(!success)
		{
			Runtime::error("Could not fetch contents of: " + dir->path());
			return;
		}
		std::scoped_lock l(_directoryLock);
		auto oldChildren = std::move(dir->children);
		dir->children.resize(0);
		dir->files.resize(0);

		std::vector<std::string> directories;
		sData >> directories >> dir->files;
		for(auto& d : directories)
		{
			bool exists = false;
			for (auto& od : oldChildren)
			{
				if(od && od->name == d)
				{
					exists = true;
					dir->children.push_back(std::move(od));
					break;
				}
			}
			if(!exists)
			{
				auto newDir = std::make_unique<Directory>();
				newDir->name = d;
				newDir->parent = dir;
				fetchDirectory(newDir.get());
				dir->children.push_back(std::move(newDir));
			}
		}

		dir->loaded = true;
	});
}

void AssetBrowserWindow::moveDirectory(Directory* target, Directory* destination)
{
	assert(target != &_root);
	if(target->parent == destination) //Don't do anything if we're already in the state we want
		return;
	if(destination->hasParent(target)) //We can't move a parent folder into a child
		return;
	net::Request req("moveDirectory");
	req.body() << target->path() << (destination->path() + target->name);

	Runtime::getModule<Editor>()->server()->sendRequest(req).then([this, target, destination](ISerializedData sData) {
        bool success;
        sData >> success;
        if (!success)
        {
			Runtime::error("Could not move file");
	        return;
        }
		std::scoped_lock l(_directoryLock);
		auto& children = target->parent->children;
		std::unique_ptr<Directory> movedDir;
		for(auto i = children.begin(); i != children.end(); i++){
			if(i->get() == target)
			{
				movedDir = std::move(*i);
				children.erase(i);
				break;
			}
		}
		assert(movedDir);
		target->parent = destination;
		destination->children.push_back(std::move(movedDir));
		std::sort(destination->children.begin(), destination->children.end(), [](auto& a, auto& b){
			return strCaseCompare(a->name, b->name);
		});
    });
}

void AssetBrowserWindow::importAsset()
{
	assert(_currentDir);
	net::Request req("processAsset");
	auto lastSlash = std::max(_filePath.find_last_of('/'), _filePath.find_last_of('\\'));
	if(lastSlash == std::string::npos)
		lastSlash = 0;
	std::string assetFilename = _filePath.substr(lastSlash);
	std::string assetPath = _currentDir->path();

	std::string assetData;
	FileManager::readFile(_filePath, assetData);

	req.body() << assetFilename << _name << assetPath << assetData;

	Directory* dir = _currentDir;
	Runtime::getModule<Editor>()->server()->sendRequest(req).then([this, dir](ISerializedData sData)
    {
        bool success;
        sData >> success;
        if (!success)
        {
			Runtime::error("Could not import asset");
			return;
		}
	    fetchDirectory(dir);
    });
}


std::string AssetBrowserWindow::Directory::path() const
{
	if(!parent)
		return "";
	else
		return parent->path() + name + "/";
}

bool AssetBrowserWindow::Directory::hasParent(AssetBrowserWindow::Directory* dir) const
{
	if(!parent)
		return false;
	if(parent == dir)
		return true;
	return parent->hasParent(dir);
}

void AssetBrowserWindow::Directory::setParentsOpen()
{
	if(!parent)
		return;
	parent->open = true;
	parent->setParentsOpen();

}
