//
// Created by wirewhiz on 20/07/22.
//

#include "assetBrowserWidget.h"
#include "imgui.h"
#include "ui/IconsFontAwesome6.h"
#include "utility/strCaseCompare.h"
#include "fileManager/fileManager.h"
#include "assets/assetManager.h"
#include "editor/editorEvents.h"
#include "editor/windows/createAssetWindow.h"
#include "editor/serverFilesystem.h"
#include "ecs/entity.h"
#include "graphics/graphics.h"


AssetBrowserWidget::AssetBrowserWidget(GUI &ui, bool allowEdits) : _ui(ui), _allowEdits(allowEdits), _fs(*Runtime::getModule<ServerFilesystem>())
{
    setDirectory(_fs.root());
}

void AssetBrowserWidget::displayDirectoryTree()
{
    displayDirectoriesRecursive(_fs.root());
}

void AssetBrowserWidget::displayDirectoriesRecursive(ServerDirectory* dir)
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
        if (dir != _fs.root() && ImGui::BeginDragDropSource())
        {
            std::string path = dir->path();
            ImGui::SetDragDropPayload("directory", &dir, sizeof(ServerDirectory*));
            ImGui::Text("%s", dir->name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("directory"))
            {
                _fs.moveDirectory(*(ServerDirectory**) p->Data, dir);
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

void AssetBrowserWidget::setDirectory(ServerDirectory* dir)
{
    if(dir == _currentDir)
        return;
    _currentDir = dir;
    _currentDir->setParentsOpen();
    if(!dir->loaded)
    {
        _fs.fetchDirectory(dir);
    }
}

void AssetBrowserWidget::reloadCurrentDirectory()
{
    _fs.fetchDirectory(_currentDir);
}

void AssetBrowserWidget::displayFiles()
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
                        _fs.deleteDirectory(dir.get());
                    }
                    if (ImGui::Selectable(ICON_FA_ARROWS_ROTATE "Refresh"))
                    {
                        _fs.fetchDirectory(_currentDir);
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropSource())
                {
                    std::string path = dir->path();
                    auto* dirPtr = dir.get();
                    ImGui::SetDragDropPayload("directory", &dirPtr, sizeof(ServerDirectory*));
                    ImGui::Text("%s",dir->name.c_str());
                    ImGui::EndDragDropSource();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if(const ImGuiPayload* p = ImGui::AcceptDragDropPayload("directory"))
                    {
                        _fs.moveDirectory(*(ServerDirectory**)p->Data, dir.get());
                    }
                }
            }
            ImGui::PopID();
        }
        for (auto& file: _currentDir->files)
        {
            ImGui::PushID(file.name.c_str());
            if (ImGui::Selectable(std::string(ICON_FA_FILE " " + file.name).c_str()))
            {
                if(file.isAsset)
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
                }
            }
            if(ImGui::IsItemHovered())
            {
                fileHovered = true;
                if(file.isAsset)
                    ImGui::SetTooltip("ID: %s", file.assetID.string().c_str());//Slow, cache string somewhere in future.
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            {
                ImGui::OpenPopup("fileActions");
            }

            if (ImGui::BeginPopup("fileActions"))
            {
                ImGui::TextDisabled(ICON_FA_TRASH "Delete");
                if (ImGui::Selectable(ICON_FA_ARROWS_ROTATE "Refresh"))
                {
                    _fs.fetchDirectory(_currentDir);
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
            {
                _ui.openPopup(std::make_unique<CreateDirectoryPopup>(_currentDir, _fs));
            }
            if (ImGui::Selectable(ICON_FA_FILE_IMPORT " Import Asset"))
            {
                _ui.addWindow<CreateAssetWindow>(_currentDir);
            }
            if (ImGui::Selectable(ICON_FA_ARROWS_ROTATE "Refresh"))
            {
                _fs.fetchDirectory(_currentDir);
            }
            ImGui::EndPopup();
        }
    }
}


ServerDirectory* AssetBrowserWidget::currentDirectory()
{
    return _currentDir;
}

void AssetBrowserWidget::displayFullBrowser()
{
    ImGui::TextDisabled("/%s", _currentDir->path().c_str());

    if(_allowEdits)
    {
        /* When an asset is selected, we create an AssetEditorContext object, and that creates/destroys entities. This
         * should really only be done on the main thread, as otherwise we might get race conditions. As we sometimes
         * need to request an asset, the responses may come in on another thread and we need to switch back to main*/
        while(!_mainThreadActions.empty())
            _mainThreadActions.pop_front()();
    }

    if(_currentDir != _fs.root())
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

CreateDirectoryPopup::CreateDirectoryPopup(ServerDirectory* parent, ServerFilesystem& fs) : _parent(parent), GUIPopup("Create Directory"), _fs(fs)
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
        _fs.createDirectory(_parent, _dirName);
    }
}
