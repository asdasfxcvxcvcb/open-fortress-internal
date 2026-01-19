#pragma once

#include "../Hooks/Hooks.h"
#include "../Util/DirectX/DirectX.h"

class CGlobal_Entry
{
public:
	void Load();
	void Unload();
};

namespace G { inline CGlobal_Entry Entry; }