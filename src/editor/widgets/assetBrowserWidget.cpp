//
// Created by wirewhiz on 20/07/22.
//

#include "assetBrowserWidget.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "ui/IconsFontAwesome6.h"
#include "utility/strCaseCompare.h"
#include "fileManager/fileManager.h"
#include "assets/assetManager.h"
#include "editor/editorEvents.h"
//#include "editor/windows/createAssetWindow.h"
#include "ecs/entity.h"
#include "graphics/graphics.h"
#include "editor/editor.h"


AssetBrowserWidget::AssetBrowserWidget(GUI &ui, bool allowEdits) : _ui(ui), _allowEdits(allowEdits)
{
	_rootPath = Runtime::getModule<Editor>()->project().projectDirectory() / "assets";
	_root = FileManager::getDirectoryTree(_rootPath);
	setDirectory(_root.get());
}

void AssetBrowserWidget::displayDirectoryTree()
{
    displayDirectoriesRecursive(_root.get());
}

void AssetBrowserWidget::displayDirectoriesRecursive(FileManager::Directory* dir)
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
    if(_allowEdits)
    {
        if (dir != _root.get() && ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("directory", &dir, sizeof(FileManager::Directory*));
            ImGui::Text("%s", dir->name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("directory"))
            {
                FileManager::moveFile(_rootPath / (*(FileManager::Directory**)p->Data)->path(), _rootPath / dir->path());
            }
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

void AssetBrowserWidget::setDirectory(FileManager::Directory* dir)
{
    _currentDir = dir;
    _currentDir->setParentsOpen();
    _contents = FileManager::getDirectoryContents(_rootPath / _currentDir->path());
	if(_contents.directories.size() != _currentDir->children.size())
	{
		_currentDir->children = std::move(FileManager::getDirectoryTree(_rootPath / _currentDir->path())->children);
		for(auto& c : _currentDir->children)
			c->parent = _currentDir;
	}
}

void AssetBrowserWidget::displayFiles()
{
    bool fileHovered = false;

    if (!_currentDir)
        ImGui::Text("No directory selected");
    else
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, {0.0f, 0.6f});
        for (auto& dir: _currentDir->children)
        {
            ImGui::PushID(dir->name.c_str());
            if (ImGui::Selectable(std::string(ICON_FA_FOLDER " " + dir->name).c_str()))
            {
                setDirectory(dir.get());
            }
            fileHovered |= ImGui::IsItemHovered();
            if(_allowEdits)
            {
                if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
                {
                    ImGui::OpenPopup("directoryActions");
                }

                if (ImGui::BeginPopup("directoryActions"))
                {
                    if (ImGui::Selectable(ICON_FA_TRASH "Delete"))
                    {
                        //Todo: confirmation dialog
                         FileManager::deleteFile(_rootPath / dir->path());
						 reloadCurrentDirectory();
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropSource())
                {
                    auto* dirPtr = dir.get();
                    ImGui::SetDragDropPayload("directory", &dirPtr, sizeof(FileManager::Directory*));
                    ImGui::Text("%s",dir->name.c_str());
                    ImGui::EndDragDropSource();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if(const ImGuiPayload* p = ImGui::AcceptDragDropPayload("directory"))
                    {
                        FileManager::moveFile(_rootPath / (*(FileManager::Directory**)p->Data)->path(), _rootPath / dir->path());
                    }
                }
            }
            ImGui::PopID();
        }
        for (auto& file: _contents.files)
        {
            ImGui::PushID(file.c_str());
            if (ImGui::Selectable(std::string(ICON_FA_FILE " " + file).c_str()))
            {
                /*if(file.isAsset)
                {
                    // Make sure to construct the focus asset event on the main thread
                    AssetManager& am = *Runtime::getModule<AssetManager>();
                    am.fetchAsset(file.assetID).then([this](Asset* asset){
                        if(asset)
                            _mainThreadActions.push_back([this, asset]{
                                _ui.sendEvent(std::make_unique<FocusAssetEvent>(asset->id));
                            });
                        else
                            Runtime::error("Selected asset was null!");
                    });
                }*/
            }
            if(ImGui::IsItemHovered())
            {
                fileHovered = true;
                /*if(file.isAsset)
                    ImGui::SetTooltip("ID: %s", file.assetID.string().c_str());//Slow, cache string somewhere in future.*/
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            {
                ImGui::OpenPopup("fileActions");
            }

            if (ImGui::BeginPopup("fileActions"))
            {
                if(ImGui::Selectable(ICON_FA_TRASH "Delete"))
                {
	                FileManager::deleteFile(_rootPath / _currentDir->path() / file);
					reloadCurrentDirectory();
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }
        ImGui::PopStyleVar(1);
    }
    assert(_currentDir);
    if(_allowEdits)
    {

        if (!fileHovered && ImGui::BeginPopupContextWindow("directoryActions"))
        {
            if (ImGui::Selectable(ICON_FA_FOLDER " New Directory"))
	            _ui.openPopup(std::make_unique<CreateDirectoryPopup>(*this));
            ImGui::EndPopup();
        }
    }
}


std::filesystem::path AssetBrowserWidget::currentDirectory()
{
    return _rootPath / _currentDir->path();
}

void AssetBrowserWidget::displayFullBrowser()
{
    ImGui::TextDisabled("/%s", _currentDir->path().string().c_str());

    if(_allowEdits)
    {
        /* When an asset is selected, we create an AssetEditorContext object, and that creates/destroys entities. This
         * should really only be done on the main thread, as otherwise we might get race conditions. As we sometimes
         * need to request an asset, the responses may come in on another thread and we need to switch back to main*/
        while(!_mainThreadActions.empty())
            _mainThreadActions.pop_front()();
    }

    if(_currentDir != _root.get())
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
        displayDirectoryTree();
        ImGui::EndChild();
        ImGui::TableNextColumn();
        ImGui::BeginChild("Directory Contents", {0,0}, false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        displayFiles();
        ImGui::EndChild();
        ImGui::EndTable();
    }
}

void AssetBrowserWidget::reloadCurrentDirectory()
{
	setDirectory(_currentDir);
}

CreateDirectoryPopup::CreateDirectoryPopup(AssetBrowserWidget& widget) : _widget(widget), GUIPopup("Create Directory")
{
}

void CreateDirectoryPopup::drawBody()
{
    ImGui::Text("Create Directory:");
    if (ImGui::InputText("##name", &_dirName, ImGuiInputTextFlags_AutoSelectAll
        | ImGuiInputTextFlags_EnterReturnsTrue)
        || ImGui::Button("create"))
    {
        ImGui::CloseCurrentPopup();
        FileManager::createDirectory(_widget.currentDirectory() / _dirName);
		_widget.reloadCurrentDirectory();
    }
}
