//
// Created by eli on 5/21/2022.
//

#ifndef BRANEENGINE_ENTITIESWINDOW_H
#define BRANEENGINE_ENTITIESWINDOW_H

#include "../editorWindow.h"

class EntitiesWindow : public EditorWindow
{
public:
	EntitiesWindow(EditorUI& ui);
	void draw() override;
};


#endif //BRANEENGINE_ENTITIESWINDOW_H
