//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_DATAWINDOW_H
#define BRANEENGINE_DATAWINDOW_H

#include "editorWindow.h"
#include <memory>

#include "ecs/entityID.h"

class EditorAsset;
class Assembly;

class DataWindow : public EditorWindow
{
	enum class FocusMode{
		asset,
		entity
	};
	FocusMode _focusMode;
	std::shared_ptr<EditorAsset> _focusedAsset;
	size_t _focusedAssetEntity = 0;

    struct DraggedComponent
    {
        Assembly* asset;
        size_t entity;
        size_t componentIndex;
    };

	EntityID _focusedEntity;
	void displayAssetData();
	void displayEntityData();
	void displayEntityAssetData();

	void displayAssemblyData();
	void displayMeshData();
    void displayMaterialData();
    void displayContent() override;
public:
    DataWindow(GUI& ui, Editor& editor);
};


#endif //BRANEENGINE_DATAWINDOW_H
