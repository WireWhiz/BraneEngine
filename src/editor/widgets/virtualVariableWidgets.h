//
// Created by wirewhiz on 21/07/22.
//

#ifndef BRANEENGINE_VIRTUALVARIABLEWIDGETS_H
#define BRANEENGINE_VIRTUALVARIABLEWIDGETS_H

#include "common/ecs/component.h"

enum class UiChangeType
{
    none = 0,
    ongoing = 1,
    finished = 2
};

class VirtualVariableWidgets
{
public:
    static UiChangeType displayVirtualComponentData(VirtualComponentView component);
    static UiChangeType displayVirtualVariable(const char* name, VirtualType::Type type, byte* data);
};


#endif //BRANEENGINE_VIRTUALVARIABLEWIDGETS_H
