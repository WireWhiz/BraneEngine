//
// Created by eli on 7/13/2022.
//

#include "editorEvents.h"

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