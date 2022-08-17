//
// Created by wirewhiz on 20/07/22.
//

#ifndef BRANEENGINE_ASSETBROWSERWIDGET_H
#define BRANEENGINE_ASSETBROWSERWIDGET_H

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "ui/gui.h"
#include "ui/guiPopup.h"
#include "utility/asyncQueue.h"
#include "fileManager/fileManager.h"


class AssetBrowserWidget
{
private:
	std::filesystem::path _rootPath;
	std::unique_ptr<FileManager::Directory> _root;
	FileManager::Directory* _currentDir;
    int _selectedFile = -1;

	FileManager::DirectoryContents _contents;

    GUI& _ui;
    bool _allowEdits;
    AsyncQueue<std::function<void()>> _mainThreadActions;

    void displayDirectoriesRecursive(FileManager::Directory* dir);
public:
    AssetBrowserWidget(GUI &ui, bool allowEdits);
    void displayDirectoryTree();
    void displayFiles();
    void displayFullBrowser();
	std::filesystem::path currentDirectory();
    void reloadCurrentDirectory();
    void setDirectory(FileManager::Directory* dir);
};

class CreateDirectoryPopup : public GUIPopup
{
	AssetBrowserWidget& _widget;
	std::string _dirName = "new dir";
	void drawBody() override;
public:
	CreateDirectoryPopup(AssetBrowserWidget& widget);
};

#endif //BRANEENGINE_ASSETBROWSERWIDGET_H
