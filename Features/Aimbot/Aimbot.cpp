#include "Aimbot.h"
#include "../Vars.h"

bool CAimbot::GetHitbox(C_TFPlayer* pEntity, Vector& vOut)
{
	if (!pEntity)
		return false;

	matrix3x4_t BoneMatrix[128];
	if (!pEntity->SetupBones(BoneMatrix, 128, 0x100, I::GlobalVarsBase->curtime))
		return false;

	int nHitbox = 3; // Default to body

	// Auto hitbox selection based on weapon
	if (Vars::Aimbot::Hitbox == 2) // Auto mode
	{
		auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
		if (pLocal)
		{
			C_TFPlayer* pLocalPlayer = reinterpret_cast<C_TFPlayer*>(pLocal);
			C_BaseCombatWeapon* pWeapon = pLocalPlayer->GetActiveWeapon();
			
			if (pWeapon)
			{
				const char* weaponName = pWeapon->GetName();
				
				// Check weapon name for railgun or sniper rifle
				bool isRailgun = (strstr(weaponName, "railgun") != nullptr || strstr(weaponName, "RAILGUN") != nullptr);
				bool isSniper = (strstr(weaponName, "sniperrifle") != nullptr || strstr(weaponName, "SNIPERRIFLE") != nullptr);
				
				// Only aim for head with railgun and sniper rifle
				if (isRailgun || isSniper)
					nHitbox = 0; // Head for sniper weapons
			}
		}
	}
	else
	{
		// Manual hitbox selection: 0 = Head, 1 = Body
		nHitbox = (Vars::Aimbot::Hitbox == 0) ? 0 : 3;
	}

	const auto pModel = pEntity->GetModel();
	if (!pModel)
		return false;

	const auto pHdr = I::ModelInfoClient->GetStudiomodel(pModel);
	if (!pHdr)
		return false;

	const auto pSet = pHdr->pHitboxSet(pEntity->m_nHitboxSet());
	if (!pSet)
		return false;

	const auto pBox = pSet->pHitbox(nHitbox);
	if (!pBox)
		return false;

	Vector vMin, vMax;
	U::Math.VectorTransform(pBox->bbmin, BoneMatrix[pBox->bone], vMin);
	U::Math.VectorTransform(pBox->bbmax, BoneMatrix[pBox->bone], vMax);

	vOut = (vMin + vMax) * 0.5f;
	return true;
}

bool CAimbot::IsVisible(C_TFPlayer* pLocal, C_TFPlayer* pEntity, const Vector& vPos)
{
	trace_t trace;
	CTraceFilterHitscan filter;
	filter.pSkip = pLocal;

	const Vector vLocalPos = pLocal->GetShootPos();

	U::Trace.TraceRay(vLocalPos, vPos, (MASK_SHOT | CONTENTS_GRATE), &filter, &trace);

	return (trace.m_pEnt == pEntity || trace.fraction > 0.99f);
}

bool CAimbot::IsValidTarget(C_TFPlayer* pLocal, C_TFPlayer* pEntity)
{
	if (!pEntity || pEntity == pLocal)
		return false;

	if (pEntity->deadflag())
		return false;

	if (pEntity->m_iHealth() <= 0)
		return false;

	// Check if player is marked as friend
	if (Vars::Aimbot::IgnoreFriends)
	{
		player_info_t playerInfo;
		if (I::EngineClient->GetPlayerInfo(pEntity->entindex(), &playerInfo))
		{
			if (Vars::CustomFriends.count(playerInfo.friendsID) > 0)
				return false;
		}
	}

	// Team check - skip if FFA mode is enabled
	if (!Vars::Aimbot::FFAMode)
	{
		if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
			return false;
	}

	if (pEntity->IsDormant())
		return false;

	// Check cloaked
	if (Vars::Aimbot::IgnoreCloaked)
	{
		if (pEntity->InCond(4)) // TF_COND_STEALTHED
			return false;
	}

	// Check invulnerable
	if (Vars::Aimbot::IgnoreInvulnerable)
	{
		if (pEntity->InCondUber() || pEntity->InCondShield())
			return false;
	}

	return true;
}

AimbotTarget CAimbot::GetBestTarget(C_TFPlayer* pLocal, CUserCmd* pCmd)
{
	AimbotTarget bestTarget;
	bestTarget.flFOV = Vars::Aimbot::FOV;

	const Vector vLocalPos = pLocal->GetShootPos();
	const Vector vForward = U::Math.GetAngleToPosition(vLocalPos, vLocalPos + Vector(1, 0, 0));

	int validTargets = 0;
	int totalPlayers = 0;

	for (int i = 1; i <= I::GlobalVarsBase->maxClients; i++)
	{
		auto pEntity = I::ClientEntityList->GetClientEntity(i);
		if (!pEntity)
			continue;

		totalPlayers++;
		C_TFPlayer* pPlayer = reinterpret_cast<C_TFPlayer*>(pEntity);
		
		if (!IsValidTarget(pLocal, pPlayer))
			continue;

		validTargets++;

		Vector vHitbox;
		if (!GetHitbox(pPlayer, vHitbox))
			continue;

		// Check if preferred hitbox is visible
		bool bPreferredVisible = IsVisible(pLocal, pPlayer, vHitbox);
		Vector vPreferredHitbox = vHitbox;
		
		// If in auto mode, also check alternate hitbox
		Vector vAlternateHitbox;
		bool bAlternateVisible = false;
		
		if (Vars::Aimbot::Hitbox == 2)
		{
			matrix3x4_t BoneMatrix[128];
			if (pPlayer->SetupBones(BoneMatrix, 128, 0x100, I::GlobalVarsBase->curtime))
			{
				const auto pModel = pPlayer->GetModel();
				if (pModel)
				{
					const auto pHdr = I::ModelInfoClient->GetStudiomodel(pModel);
					if (pHdr)
					{
						const auto pSet = pHdr->pHitboxSet(pPlayer->m_nHitboxSet());
						if (pSet)
						{
							// Determine alternate hitbox
							int nAlternateHitbox = 3;
							
							auto pWeapon = pLocal->GetActiveWeapon();
							if (pWeapon)
							{
								const char* weaponName = pWeapon->GetName();
								bool isRailgun = (strstr(weaponName, "railgun") != nullptr || strstr(weaponName, "RAILGUN") != nullptr);
								bool isSniper = (strstr(weaponName, "sniperrifle") != nullptr || strstr(weaponName, "SNIPERRIFLE") != nullptr);
								
								// If weapon prefers head, alternate is body. If weapon prefers body, alternate is head
								if (isRailgun || isSniper)
									nAlternateHitbox = 3; // Preferred head, alternate body
								else
									nAlternateHitbox = 0; // Preferred body, alternate head
							}
							
							const auto pBox = pSet->pHitbox(nAlternateHitbox);
							if (pBox)
							{
								Vector vMin, vMax;
								U::Math.VectorTransform(pBox->bbmin, BoneMatrix[pBox->bone], vMin);
								U::Math.VectorTransform(pBox->bbmax, BoneMatrix[pBox->bone], vMax);
								vAlternateHitbox = (vMin + vMax) * 0.5f;
								
								bAlternateVisible = IsVisible(pLocal, pPlayer, vAlternateHitbox);
							}
						}
					}
				}
			}
		}
		
		// Decide which hitbox to use: ALWAYS prefer the weapon's optimal hitbox if visible
		Vector vFinalHitbox;
		bool bHasValidHitbox = false;
		
		if (bPreferredVisible)
		{
			// Preferred hitbox is visible - use it!
			vFinalHitbox = vPreferredHitbox;
			bHasValidHitbox = true;
		}
		else if (bAlternateVisible)
		{
			// Only use alternate if preferred is not visible
			vFinalHitbox = vAlternateHitbox;
			bHasValidHitbox = true;
		}
		
		if (!bHasValidHitbox)
			continue;

		const float flFOV = U::Math.GetFovBetween(pCmd->viewangles, U::Math.GetAngleToPosition(vLocalPos, vFinalHitbox));
		const float flDistance = vLocalPos.DistTo(vFinalHitbox);

		// Check FOV only if using FOV-based target selection
		if (Vars::Aimbot::TargetSelection == 1 && flFOV > Vars::Aimbot::FOV)
			continue;

		bool bBetter = false;

		switch (Vars::Aimbot::TargetSelection)
		{
		case 0: // Distance - ignore FOV, just get closest
			bBetter = (bestTarget.pEntity == nullptr || flDistance < bestTarget.flDistance);
			break;
		case 1: // FOV - already checked above
			bBetter = (bestTarget.pEntity == nullptr || flFOV < bestTarget.flFOV);
			break;
		}

		if (bBetter)
		{
			bestTarget.pEntity = pPlayer;
			bestTarget.vPos = vFinalHitbox;
			bestTarget.flFOV = flFOV;
			bestTarget.flDistance = flDistance;
		}
	}

	return bestTarget;
}

void CAimbot::AimAt(C_TFPlayer* pLocal, CUserCmd* pCmd, const Vector& vTarget)
{
	const Vector vLocalPos = pLocal->GetShootPos();
	Vector vAngle = U::Math.GetAngleToPosition(vLocalPos, vTarget);

	switch (Vars::Aimbot::Mode)
	{
	case 0: // Plain
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(pCmd->viewangles);
		break;

	case 1: // Smooth
	{
		Vector vDelta = vAngle - pCmd->viewangles;
		U::Math.ClampAngles(vDelta);
		pCmd->viewangles += vDelta / Vars::Aimbot::SmoothAmount;
		I::EngineClient->SetViewAngles(pCmd->viewangles);
		break;
	}

	case 2: // Silent (PSilent)
		pCmd->viewangles = vAngle;
		G::bPSilentAngles = true;
		break;
	}

	U::Math.ClampAngles(pCmd->viewangles);
}

void CAimbot::Run(C_TFPlayer* pLocal, CUserCmd* pCmd)
{
	// Reset active state at the start of each frame
	G::bAimbotActive = false;

	if (!Vars::Aimbot::Enabled)
		return;

	if (!pLocal || !pLocal->IsAlive())
		return;

	// Check aimbot key
	bool bKeyPressed = false;
	
	if (Vars::Aimbot::AimbotKey == 0)
	{
		bKeyPressed = true; // Always on
	}
	else if (Vars::Aimbot::AimbotKey == VK_LBUTTON)
	{
		bKeyPressed = (pCmd->buttons & IN_ATTACK) || (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	}
	else if (Vars::Aimbot::AimbotKey == VK_RBUTTON)
	{
		bKeyPressed = (pCmd->buttons & IN_ATTACK2) || (GetAsyncKeyState(VK_RBUTTON) & 0x8000);
	}
	else
	{
		bKeyPressed = (GetAsyncKeyState(Vars::Aimbot::AimbotKey) & 0x8000);
	}

	if (!bKeyPressed)
		return;

	auto target = GetBestTarget(pLocal, pCmd);
	
	if (!target.pEntity)
		return;

	// We have a valid target - set active state
	G::bAimbotActive = true;

	AimAt(pLocal, pCmd, target.vPos);

	// Auto shoot if enabled
	if (Vars::Aimbot::AutoShoot)
	{
		pCmd->buttons |= IN_ATTACK;
	}
}

void CAimbot::DrawFOV()
{
	// Only draw FOV circle when using FOV-based target selection
	if (!Vars::Aimbot::Enabled || !Vars::Aimbot::DrawFOV || Vars::Aimbot::TargetSelection != 1)
		return;

	// Get screen center
	int centerX = H::Draw.m_nScreenW / 2;
	int centerY = H::Draw.m_nScreenH / 2;

	// Calculate radius based on FOV
	float fovRadians = (Vars::Aimbot::FOV * 3.14159f) / 180.0f;
	int radius = static_cast<int>((H::Draw.m_nScreenH / 2.0f) * tanf(fovRadians / 2.0f));

	// Draw the FOV circle
	Color circleColor = Color(255, 255, 255, 100);
	H::Draw.OutlinedCircle(centerX, centerY, radius, 64, circleColor);
}
