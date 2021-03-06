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

FocusAssetEvent::FocusAssetEvent(Asset* asset) : GUIEvent("focus asset")
{
	_asset = asset;
}

Asset* FocusAssetEvent::asset() const
{
	return _asset;
}

FocusEntityAssetEvent::FocusEntityAssetEvent(size_t index) : GUIEvent("focus entity asset")
{
	_index = index;
}
size_t FocusEntityAssetEvent::entity() const
{
	return _index;
}