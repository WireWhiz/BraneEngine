//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_ENTITIESWINDOW_H
#define BRANEENGINE_ENTITIESWINDOW_H

#include <ui/guiWindow.h>
#include <memory>

class AssetEditorContext;
class EntityManager;
class Assembly;
class EntitiesWindow : public GUIWindow
{
    std::shared_ptr<AssetEditorContext> _assetCtx;
	EntityManager* _em;
    void displayAssemblyEntities(Assembly* assembly, size_t entIndex);
public:
	EntitiesWindow(GUI& ui);
	void draw() override;
};


#endif //BRANEENGINE_ENTITIESWINDOW_H
