#pragma once

#include "../../../SDK/SDK.h"
#include "../../Vars.h"

// Projectile aimbot - for rockets, grenades, arrows, etc.
// TODO: Implement projectile prediction and leading
class CAimbotProjectile
{
public:
	void Run(C_TFPlayer* pLocal, CUserCmd* pCmd);
};

namespace F { inline CAimbotProjectile AimbotProjectile; }
