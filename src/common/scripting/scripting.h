//
// Created by eli on 12/9/2022.
//

#ifndef BRANEENGINE_SCRIPTING_H
#define BRANEENGINE_SCRIPTING_H

#include "runtime/runtime.h"
#include "scripting-runtime.h"

class ScriptManager : public Module
{

public:
    ScriptManager();
    static const char* name();
};


#endif //BRANEENGINE_SCRIPTING_H
