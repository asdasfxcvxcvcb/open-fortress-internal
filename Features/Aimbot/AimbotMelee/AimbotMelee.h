#pragma once

#include "../../../SDK/SDK.h"
#include "../../Vars.h"

// Melee aimbot - for crowbar, knife, axe, etc.
// TODO: Implement melee-specific targeting
class CAimbotMelee
{
public:
	void Run(C_TFPlayer* pLocal, CUserCmd* pCmd);
};

namespace F { inline CAimbotMelee AimbotMelee; }
