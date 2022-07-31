//
// Created by eli on 7/13/2022.
//

#ifndef BRANEENGINE_EDITOREVENTS_H
#define BRANEENGINE_EDITOREVENTS_H

#include <cstdint>
#include "common/ui/guiEvent.h"
#include "ecs/entityID.h"
#include <memory>
namespace net {
    class Connection;
}
class Asset;
class ServerDirectory;
class AssetEditorContext;

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
	std::shared_ptr<AssetEditorContext> _asset;
public:
	FocusAssetEvent(Asset* asset);
    std::shared_ptr<AssetEditorContext> asset() const;
};

class FocusEntityEvent : public GUIEvent
{
    EntityID _id;
public:
    FocusEntityEvent(EntityID id);
    EntityID id() const;
};

class FocusEntityAssetEvent : public GUIEvent
{
	size_t _index;
public:
	FocusEntityAssetEvent(size_t index);
	size_t entity() const;
};

#endif //BRANEENGINE_EDITOREVENTS_H
