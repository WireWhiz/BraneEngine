//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_DATAWINDOW_H
#define BRANEENGINE_DATAWINDOW_H

#include <ui/guiWindow.h>

#include "ecs/entityID.h"

class Asset;

class DataWindow : public GUIWindow
{
	enum class FocusMode{
		asset,
		entity
	};
	FocusMode _focusMode;
	Asset* _focusedAsset = nullptr;
	size_t _focusedAssetEntity = 0;

	EntityID _focusedEntity;
	void displayAssetData();
	void displayEntityData();
	void displayEntityAssetData();

	void displayAssemblyData();
	void displayMeshData();
public:
	DataWindow(GUI& ui, GUIWindowID id);
	void draw() override;
};


#endif //BRANEENGINE_DATAWINDOW_H
