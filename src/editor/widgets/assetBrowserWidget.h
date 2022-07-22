//
// Created by wirewhiz on 20/07/22.
//

#ifndef BRANEENGINE_ASSETBROWSERWIDGET_H
#define BRANEENGINE_ASSETBROWSERWIDGET_H

#include <string>
#include <vector>
#include <memory>

#include "assets/assetID.h"
#include "ui/gui.h"
#include "editor/serverFilesystem.h"

class CreateDirectoryPopup : public GUIPopup
{
    ServerFilesystem& _fs;
    ServerDirectory* _parent;
    std::string _dirName = "new dir";
    void drawBody() override;
public:
    CreateDirectoryPopup(ServerDirectory* parent, ServerFilesystem& fs);
};

class AssetBrowserWidget
{
public:
    enum Mode{
        selectDirectory,
        selectFile,
        browse
    };
private:
    ServerDirectory* _currentDir = nullptr;

    ServerFilesystem& _fs;
    GUI& _ui;
    Mode _mode;

    void displayDirectoriesRecursive(ServerDirectory* dir);
public:
    AssetBrowserWidget(GUI &ui, AssetBrowserWidget::Mode mode);
    void displayDirectoryTree();
    void displayFiles();
    void displayFullBrowser();
    std::string currentPath();
    void reloadCurrentDirectory();
    void setDirectory(ServerDirectory* dir);
};


#endif //BRANEENGINE_ASSETBROWSERWIDGET_H
