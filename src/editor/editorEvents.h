//
// Created by eli on 7/13/2022.
//

#ifndef BRANEENGINE_EDITOREVENTS_H
#define BRANEENGINE_EDITOREVENTS_H

#include "common/assets/asset.h"
#include "common/ui/guiEvent.h"
#include <cstddef>
#include "networking/connection.h"
#include "serverFilesystem.h"

class LoginEvent : public GUIEvent
{
    net::Connection* _server;
public:
    LoginEvent(net::Connection* server);
    inline net::Connection* server() const {return _server;}
};

class DirectoryUpdateEvent : public GUIEvent
{
    ServerDirectory* _dir;
public:
    DirectoryUpdateEvent(ServerDirectory* dir);
    ServerDirectory* directory() const;
};

class FocusAssetEvent : public GUIEvent
{
	Asset* _asset;
public:
	FocusAssetEvent(Asset* asset);
	Asset* asset() const;
};

class FocusEntityAssetEvent : public GUIEvent
{
	size_t _index;
public:
	FocusEntityAssetEvent(size_t index);
	size_t entity() const;
};

#endif //BRANEENGINE_EDITOREVENTS_H
