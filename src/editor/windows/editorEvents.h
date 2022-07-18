//
// Created by eli on 7/13/2022.
//

#ifndef BRANEENGINE_EDITOREVENTS_H
#define BRANEENGINE_EDITOREVENTS_H

#include <assets/asset.h>
#include <ui/guiEvent.h>
#include <cstddef>

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
