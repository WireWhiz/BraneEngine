//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_ENTITIESWINDOW_H
#define BRANEENGINE_ENTITIESWINDOW_H

#include <ui/guiWindow.h>

class Assembly;
class EntityManager;
class EntitiesWindow : public GUIWindow
{
	Assembly* _assembly = nullptr;
	EntityManager* _em;
	void displayAssemblyEntities(Assembly* assembly, size_t entIndex);
public:
	EntitiesWindow(GUI& ui, GUIWindowID id);
	void draw() override;
};


#endif //BRANEENGINE_ENTITIESWINDOW_H
