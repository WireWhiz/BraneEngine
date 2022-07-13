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
		bool open = false;
		std::string name;
		Directory* parent = nullptr;
		std::vector<std::string> files;
		std::vector<std::unique_ptr<Directory>> children;
		std::string path() const;
		bool hasParent(Directory* dir) const;
		void setParentsOpen();
	};

	std::mutex _directoryLock;
	Directory _root;
	Directory* _currentDir = nullptr;

	//gui popup context variables
	std::string _name;
	std::string _filePath;

	void displayDirectories();
	void displayDirectoriesRecursive(Directory* dir);
	void displayFiles();
	void updateStrPath();
	void reloadCurrentDirectory();
	void setDirectory(Directory* dir);
	void fetchDirectory(Directory* dir);
	void createDirectory();
	void deleteFile(const std::string& path);
	void moveDirectory(Directory* target, Directory* destination);
	void importAsset();
public:
	AssetBrowserWindow(GUI& ui, GUIWindowID id);
	void draw() override;
};


#endif //BRANEENGINE_ASSETBROWSERWINDOW_H
