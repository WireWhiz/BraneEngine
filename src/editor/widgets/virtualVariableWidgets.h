//
// Created by wirewhiz on 21/07/22.
//

#ifndef BRANEENGINE_VIRTUALVARIABLEWIDGETS_H
#define BRANEENGINE_VIRTUALVARIABLEWIDGETS_H

#include <ecs/core/component.h>

class VirtualVariableWidgets
{
public:
    static void displayVirtualComponentData(VirtualComponentView component);
    static void displayVirtualVariable(const char* name, VirtualType::Type type, byte* data);
};


#endif //BRANEENGINE_VIRTUALVARIABLEWIDGETS_H
