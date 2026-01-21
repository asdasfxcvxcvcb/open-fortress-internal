#include "AimbotHitscan.h"
#include "../Aimbot.h"
#include "../../Vars.h"
#include "../../../SDK/Helpers.h"
#include "../../../SDK/Helpers/Helpers.h"

bool CAimbotHitscan::GetHitbox(C_TFPlayer* pEntity, Vector& vOut)
{
	if (!pEntity)
		return false;

	matrix3x4_t BoneMatrix[128];
	if (!pEntity->SetupBones(BoneMatrix, 128, 0x100, I::GlobalVarsBase->curtime))
		return false;

	int nHitbox = 3;

	if (Vars::Aimbot::Hitbox == 2)
	{
		auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
		if (pLocal)
		{
			C_TFPlayer* pLocalPlayer = reinterpret_cast<C_TFPlayer*>(pLocal);
			C_BaseCombatWeapon* pWeapon = pLocalPlayer->GetActiveWeapon();
			
			if (pWeapon)
			{
				const char* weaponName = pWeapon->GetName();
				bool isRailgun = (strstr(weaponName, "railgun") != nullptr || strstr(weaponName, "RAILGUN") != nullptr);
				bool isSniper = (strstr(weaponName, "sniperrifle") != nullptr || strstr(weaponName, "SNIPERRIFLE") != nullptr);
				
				if (isRailgun || isSniper)
					nHitbox = 0;
			}
		}
	}
	else
	{
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

bool CAimbotHitscan::IsVisible(C_TFPlayer* pLocal, C_TFPlayer* pEntity, const Vector& vPos)
{
	trace_t trace;
	CTraceFilterHitscan filter;
	filter.pSkip = pLocal;

	const Vector vLocalPos = pLocal->GetShootPos();

	U::Trace.TraceRay(vLocalPos, vPos, (MASK_SHOT | CONTENTS_GRATE), &filter, &trace);

	return (trace.m_pEnt == pEntity || trace.fraction > 0.99f);
}

bool CAimbotHitscan::IsValidTarget(C_TFPlayer* pLocal, C_TFPlayer* pEntity)
{
	if (!pEntity || pEntity == pLocal)
		return false;

	if (pEntity->deadflag())
		return false;

	if (pEntity->m_iHealth() <= 0)
		return false;

	if (Vars::Aimbot::IgnoreFriends)
	{
		player_info_t playerInfo;
		if (I::EngineClient->GetPlayerInfo(pEntity->entindex(), &playerInfo))
		{
			if (Vars::CustomFriends.count(playerInfo.friendsID) > 0)
				return false;
		}
	}

	if (!Vars::Aimbot::FFAMode)
	{
		if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
			return false;
	}

	if (pEntity->IsDormant())
		return false;

	if (Vars::Aimbot::IgnoreCloaked)
	{
		if (pEntity->InCond(4))
			return false;
	}

	if (Vars::Aimbot::IgnoreInvulnerable)
	{
		if (pEntity->InCondUber() || pEntity->InCondShield())
			return false;
	}

	return true;
}

AimbotTarget CAimbotHitscan::GetBestTarget(C_TFPlayer* pLocal, CUserCmd* pCmd)
{
	AimbotTarget bestTarget;
	bestTarget.flFOV = Vars::Aimbot::FOV;

	const Vector vLocalPos = pLocal->GetShootPos();
	const Vector vForward = U::Math.GetAngleToPosition(vLocalPos, vLocalPos + Vector(1, 0, 0));

	for (int i = 1; i <= I::GlobalVarsBase->maxClients; i++)
	{
		auto pEntity = I::ClientEntityList->GetClientEntity(i);
		if (!pEntity)
			continue;

		C_TFPlayer* pPlayer = reinterpret_cast<C_TFPlayer*>(pEntity);
		
		if (!IsValidTarget(pLocal, pPlayer))
			continue;

		Vector vHitbox;
		if (!GetHitbox(pPlayer, vHitbox))
			continue;

		bool bPreferredVisible = IsVisible(pLocal, pPlayer, vHitbox);
		Vector vPreferredHitbox = vHitbox;
		
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
							int nAlternateHitbox = 3;
							
							auto pWeapon = pLocal->GetActiveWeapon();
							if (pWeapon)
							{
								const char* weaponName = pWeapon->GetName();
								bool isRailgun = (strstr(weaponName, "railgun") != nullptr || strstr(weaponName, "RAILGUN") != nullptr);
								bool isSniper = (strstr(weaponName, "sniperrifle") != nullptr || strstr(weaponName, "SNIPERRIFLE") != nullptr);
								
								if (isRailgun || isSniper)
									nAlternateHitbox = 3;
								else
									nAlternateHitbox = 0;
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
		
		Vector vFinalHitbox;
		bool bHasValidHitbox = false;
		
		if (bPreferredVisible)
		{
			vFinalHitbox = vPreferredHitbox;
			bHasValidHitbox = true;
		}
		else if (bAlternateVisible)
		{
			vFinalHitbox = vAlternateHitbox;
			bHasValidHitbox = true;
		}
		
		if (!bHasValidHitbox)
			continue;

		const float flFOV = U::Math.GetFovBetween(pCmd->viewangles, U::Math.GetAngleToPosition(vLocalPos, vFinalHitbox));
		const float flDistance = vLocalPos.DistTo(vFinalHitbox);

		if (Vars::Aimbot::TargetSelection == 1 && flFOV > Vars::Aimbot::FOV)
			continue;

		bool bBetter = false;

		switch (Vars::Aimbot::TargetSelection)
		{
		case 0:
			bBetter = (bestTarget.pEntity == nullptr || flDistance < bestTarget.flDistance);
			break;
		case 1:
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

void CAimbotHitscan::AimAt(C_TFPlayer* pLocal, CUserCmd* pCmd, const Vector& vTarget)
{
	const Vector vLocalPos = pLocal->GetShootPos();
	Vector vAngle = U::Math.GetAngleToPosition(vLocalPos, vTarget);
	const Vector vOldAngles = pCmd->viewangles;

	switch (Vars::Aimbot::Mode)
	{
	case 0:
		pCmd->viewangles = vAngle;
		U::Math.ClampAngles(pCmd->viewangles);
		I::EngineClient->SetViewAngles(pCmd->viewangles);
		break;

	case 1:
	{
		Vector vDelta = vAngle - pCmd->viewangles;
		U::Math.ClampAngles(vDelta);
		pCmd->viewangles += vDelta / Vars::Aimbot::SmoothAmount;
		U::Math.ClampAngles(pCmd->viewangles);
		I::EngineClient->SetViewAngles(pCmd->viewangles);
		break;
	}

	case 2:
	{
		auto pWeapon = pLocal->GetActiveWeapon();
		if (pWeapon)
			G::Attacking = SDK::IsAttacking(pLocal, pWeapon, pCmd);
		
		if (G::Attacking == 1)
		{
			U::Math.FixMovement(pCmd, vOldAngles, vAngle);
			pCmd->viewangles = vAngle;
			U::Math.ClampAngles(pCmd->viewangles);
			G::bSilentAngles = true;
		}
		break;
	}

	case 3:
	{
		auto pWeapon = pLocal->GetActiveWeapon();
		if (pWeapon)
			G::Attacking = SDK::IsAttacking(pLocal, pWeapon, pCmd);
		
		if (G::Attacking == 1)
		{
			U::Math.FixMovement(pCmd, vOldAngles, vAngle);
			pCmd->viewangles = vAngle;
			U::Math.ClampAngles(pCmd->viewangles);
			G::bPSilentAngles = true;
		}
		break;
	}
	}
}

void CAimbotHitscan::Run(C_TFPlayer* pLocal, CUserCmd* pCmd)
{
	G::bAimbotActive = false;

	if (!pLocal || !pLocal->IsAlive())
		return;

	auto target = GetBestTarget(pLocal, pCmd);
	
	if (!target.pEntity)
		return;

	G::bAimbotActive = true;
	F::Aimbot.SetAiming(true);

	if (Vars::Aimbot::AutoShoot)
		pCmd->buttons |= IN_ATTACK;

	AimAt(pLocal, pCmd, target.vPos);
}
