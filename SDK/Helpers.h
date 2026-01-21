#pragma once

#include "SDK.h"
#include "Globals.h"

namespace SDK
{
	// Check if weapon will fire this tick
	// Returns: 0 = not attacking, 1 = will fire this tick, 2 = queued (reloading)
	inline int IsAttacking(C_TFPlayer* pLocal, C_BaseCombatWeapon* pWeapon, CUserCmd* pCmd)
	{
		if (!pLocal || !pWeapon)
			return 0;
		
		// Not pressing attack
		if (!(pCmd->buttons & IN_ATTACK))
			return 0;
		
		// Can fire this tick
		if (G::bCanPrimaryAttack)
			return 1;
		
		// Reloading but want to fire
		if (G::bReloading)
			return 2;
		
		return 0;
	}
}

