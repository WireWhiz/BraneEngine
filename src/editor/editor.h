//
// Created by eli on 6/28/2022.
//

#ifndef BRANEENGINE_EDITOR_H
#define BRANEENGINE_EDITOR_H

#include <runtime/module.h>
#include <memory>
#include <unordered_map>
#include "assets/assetID.h"
#include "braneProject.h"
#include "utility/jsonVersioner.h"

namespace net{
    class Connection;
}

class GUI;
class GUIWindow;
class EditorAsset;
class Editor : public Module
{
	GUI* _ui;
	GUIWindow* _selectProjectWindow = nullptr;

	JsonVersionTracker _jsonTracker;
	BraneProject _project;
	void addMainWindows();
	void drawMenu();
public:
	Editor();
	void start() override;
	void loadProject(const std::filesystem::path& filepath);
	void createProject(const std::string& name, const std::filesystem::path& directory);
	BraneProject& project();
	JsonVersionTracker& jsonTracker();
    std::shared_ptr<EditorAsset> getEditorAsset(const AssetID& id);
	static const char* name();
};


#endif //BRANEENGINE_EDITOR_H
