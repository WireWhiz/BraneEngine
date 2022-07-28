//
// Created by wirewhiz on 20/07/22.
//

#ifndef BRANEENGINE_ASSETBROWSERWIDGET_H
#define BRANEENGINE_ASSETBROWSERWIDGET_H

#include <string>
#include <vector>
#include <memory>
#include "ui/gui.h"

class ServerFilesystem;
class ServerDirectory;

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
private:
    ServerDirectory* _currentDir = nullptr;
    int _selectedFile = -1;

    ServerFilesystem& _fs;
    GUI& _ui;
    bool _allowEdits;

    void displayDirectoriesRecursive(ServerDirectory* dir);
public:
    AssetBrowserWidget(GUI &ui, bool allowEdits);
    void displayDirectoryTree();
    void displayFiles();
    void displayFullBrowser();
    ServerDirectory* currentDirectory();
    void reloadCurrentDirectory();
    void setDirectory(ServerDirectory* dir);
};


#endif //BRANEENGINE_ASSETBROWSERWIDGET_H
