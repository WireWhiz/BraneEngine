//
// Created by eli on 7/13/2022.
//

#include "editorEvents.h"

LoginEvent::LoginEvent(net::Connection* server) : GUIEvent("login")
{
    _server = server;
}

DirectoryUpdateEvent::DirectoryUpdateEvent(ServerDirectory* dir) : GUIEvent("dir reload")
{
    _dir = dir;
}
ServerDirectory* DirectoryUpdateEvent::directory() const
{
    return _dir;
}

FocusAssetEvent::FocusAssetEvent(std::shared_ptr<EditorAsset> asset) : GUIEvent("focus asset")
{
	_asset = std::move(asset);
}

std::shared_ptr<EditorAsset> FocusAssetEvent::asset() const
{
	return _asset;
}

FocusEntityAssetEvent::FocusEntityAssetEvent(int index) : GUIEvent("focus entity asset")
{
	_index = index;
}

int FocusEntityAssetEvent::entity() const
{
	return _index;
}

FocusEntityEvent::FocusEntityEvent(EntityID id) : GUIEvent("focus entity")
{
    _id = id;
}

EntityID FocusEntityEvent::id() const
{
    return _id;
}

EntityAssetReloadEvent::EntityAssetReloadEvent(size_t entity) : GUIEvent("entity asset reload")
{
	_entity = entity;
}

size_t EntityAssetReloadEvent::entity() const
{
	return _entity;
}

AssetReloadEvent::AssetReloadEvent() : GUIEvent("asset reload")
{

}
