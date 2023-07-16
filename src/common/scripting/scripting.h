//
// Created by eli on 12/9/2022.
//

#ifndef BRANEENGINE_SCRIPTING_H
#define BRANEENGINE_SCRIPTING_H

#include "runtime/runtime.h"

#include "scriptRuntime/scriptRuntime.h"

class ScriptManager : public Module
{
    BraneScript::ScriptRuntime _rt;
public:
    ScriptManager();
    static const char* name();
};


#endif //BRANEENGINE_SCRIPTING_H
