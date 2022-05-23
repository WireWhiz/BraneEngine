//
// Created by eli on 5/22/2022.
//

#ifndef BRANEENGINE_ASSETBROWSERWINDOW_H
#define BRANEENGINE_ASSETBROWSERWINDOW_H

#include "../editorWindow.h"
#include "networking/networking.h"
#include <string>

class AssetBrowserWindow : public EditorWindow
{
	NetworkManager& _nm;
	std::string _strPath = "/";

	struct Directory
	{
		std::string name;
		int64_t id;
		Directory* parent;
		std::vector<std::unique_ptr<Directory>> children;
	};

	struct AssetData
	{
		int64_t id;
		std::string name;
		std::string hexIDString;
		std::string type;
		int64_t directoryID;
	};

	struct DirectoryContents
	{
		std::vector<Directory*> directories;
		std::vector<AssetData*> assets;
	};

	std::unique_ptr<Directory> _root;
	std::vector<AssetData> _directoryContents;

	Directory* _currentDir = nullptr;
	std::unique_ptr<Directory> deserializeDirectory(ISerializedData& sData, Directory* parent = nullptr);
	void displayDirectories();
	void displayDirectoriesRecursive(Directory* dir);
	void updateStrPath();
	void setDirectory(Directory* dir);
public:
	AssetBrowserWindow(EditorUI& ui);
	void draw() override;
};


#endif //BRANEENGINE_ASSETBROWSERWINDOW_H
