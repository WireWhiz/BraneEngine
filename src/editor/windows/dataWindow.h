//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_DATAWINDOW_H
#define BRANEENGINE_DATAWINDOW_H

#include <ui/guiWindow.h>
#include <memory>

#include "ecs/entityID.h"

class AssetEditorContext;

class DataWindow : public GUIWindow
{
	enum class FocusMode{
		asset,
		entity
	};
	FocusMode _focusMode;
	std::shared_ptr<AssetEditorContext> _focusedAsset;
	size_t _focusedAssetEntity = 0;

	EntityID _focusedEntity;
	void displayAssetData();
	void displayEntityData();
	void displayEntityAssetData();

	void displayAssemblyData();
	void displayMeshData();
    void displayContent() override;
public:
    DataWindow(GUI& ui);
};


#endif //BRANEENGINE_DATAWINDOW_H
