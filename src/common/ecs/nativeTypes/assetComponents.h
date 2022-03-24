#pragma once

#include "assets/assembly.h"
class AssemblyRoot : public NativeComponent<AssemblyRoot>
{
	REGISTER_MEMBERS_2("Assembly Root", id, loaded);
public:
	AssetID id;
	bool loaded;
};