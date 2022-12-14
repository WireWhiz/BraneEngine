//
// Created by eli on 12/9/2022.
//

#ifndef BRANEENGINE_SCRIPTING_H
#define BRANEENGINE_SCRIPTING_H

#include "runtime/runtime.h"

#include "scriptRuntime.h"
#include "linker.h"
#include "compiler.h"

class ScriptManager : public Module
{
    BraneScript::Linker _linker;
    BraneScript::ScriptRuntime _rt;
public:
    ScriptManager();

    BraneScript::Compiler newCompiler();
};


#endif //BRANEENGINE_SCRIPTING_H
