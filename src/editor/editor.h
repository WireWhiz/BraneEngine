//
// Created by eli on 6/28/2022.
//

#ifndef BRANEENGINE_EDITOR_H
#define BRANEENGINE_EDITOR_H

#include <runtime/module.h>
#include <memory>
#include <unordered_map>
#include "assets/assetID.h"

namespace net{
    class Connection;
}

class GUI;
class AssetEditorContext;
class Editor : public Module
{
	GUI* _ui;
	net::Connection* _server;

    // Using shared pointers so we can track which ones are in use
    std::unordered_map<AssetID, std::shared_ptr<AssetEditorContext>> _assetContexts;
	void addMainWindows();
	void drawMenu();
public:
	void start() override;
	static const char* name();
	net::Connection* server() const;
    std::shared_ptr<AssetEditorContext> getEditorContext(const AssetID& id);
};


#endif //BRANEENGINE_EDITOR_H
