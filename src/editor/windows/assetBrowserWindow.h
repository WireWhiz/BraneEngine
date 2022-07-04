//
// Created by eli on 5/22/2022.
//

#ifndef BRANEENGINE_ASSETBROWSERWINDOW_H
#define BRANEENGINE_ASSETBROWSERWINDOW_H


#include <ui/guiWindow.h>
#include "networking/networking.h"
#include <string>

class AssetBrowserWindow : public GUIWindow
{
	std::string _strPath = "";

	struct Directory
	{
		bool loaded = false;
		std::string name;
		Directory* parent = nullptr;
		std::vector<std::string> files;
		std::vector<std::unique_ptr<Directory>> children;
	};

	Directory _root;
	Directory* _currentDir = nullptr;

	std::string _newDirName;

	void displayDirectories();
	void displayDirectoriesRecursive(Directory* dir);
	void updateStrPath();
	void reloadCurrentDirectory();
	void setDirectory(Directory* dir);
	void createDirectory();
	void deleteFile(const std::string& path);
public:
	AssetBrowserWindow(GUI& ui, GUIWindowID id);
	void draw() override;
};


#endif //BRANEENGINE_ASSETBROWSERWINDOW_H
