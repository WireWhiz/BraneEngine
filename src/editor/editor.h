//
// Created by eli on 6/28/2022.
//

#ifndef BRANEENGINE_EDITOR_H
#define BRANEENGINE_EDITOR_H

#include <runtime/module.h>
#include <memory>
#include <unordered_map>
#include "assets/assetID.h"
#include "BraneProject.h"

namespace net{
    class Connection;
}

class GUI;
class AssetEditorContext;
class GUIWindow;
class Editor : public Module
{
	GUI* _ui;
	GUIWindow* _selectProjectWindow = nullptr;

    // Using shared pointers so we can track which ones are in use
    std::unordered_map<AssetID, std::shared_ptr<AssetEditorContext>> _assetContexts;
	BraneProject _project;
	void addMainWindows();
	void drawMenu();
public:
	void start() override;
	void loadProject(const std::filesystem::path& filepath);
	void createProject(const std::string& name, const std::filesystem::path& directory);
	BraneProject& project();
    std::shared_ptr<AssetEditorContext> getEditorContext(const AssetID& id);
	static const char* name();
};


#endif //BRANEENGINE_EDITOR_H
