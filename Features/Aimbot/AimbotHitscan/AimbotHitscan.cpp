#include "AimbotHitscan.h"
#include "../Aimbot.h"
#include "../../Vars.h"
#include "../../Backtrack/Backtrack.h"
#include "../../../SDK/Helpers.h"
#include "../../../SDK/Helpers/Helpers.h"


bool CAimbotHitscan::GetHitbox(C_TFPlayer* pEntity, Vector& vOut, matrix3x4_t* pBoneMatrix)
{
	if (!pEntity)
		return false;

	matrix3x4_t BoneMatrix[128];
	matrix3x4_t* pBones = pBoneMatrix;

	if (!pBones)
	{
		if (!pEntity->SetupBones(BoneMatrix, 128, 0x100, I::GlobalVarsBase->curtime))
			return false;
		pBones = BoneMatrix;
	}

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
	U::Math.VectorTransform(pBox->bbmin, pBones[pBox->bone], vMin);
	U::Math.VectorTransform(pBox->bbmax, pBones[pBox->bone], vMax);

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

AimbotTarget CAimbotHitscan::GetBestTarget(C_TFPlayer* pLocal, CUserCmd* pCmd)
{
	AimbotTarget bestTarget;
	bestTarget.flFOV = Vars::Aimbot::FOV;

	const Vector vLocalPos = pLocal->GetShootPos();
	
	// Cache weapon check for auto hitbox
	bool isRailgunOrSniper = false;
	if (Vars::Aimbot::Hitbox == 2)
	{
		auto pWeapon = pLocal->GetActiveWeapon();
		if (pWeapon)
		{
			const char* weaponName = pWeapon->GetName();
			isRailgunOrSniper = (strstr(weaponName, "railgun") != nullptr || strstr(weaponName, "sniperrifle") != nullptr);
		}
	}

	for (int i = 1; i <= I::GlobalVarsBase->maxClients; i++)
	{
		auto pEntity = I::ClientEntityList->GetClientEntity(i);
		if (!pEntity)
			continue;

		C_TFPlayer* pPlayer = reinterpret_cast<C_TFPlayer*>(pEntity);
		
		if (!F::Aimbot.IsValidTarget(pLocal, pPlayer))
			continue;

		// Helper lambda to check a specific matrix/simtime
		auto CheckTarget = [&](const BacktrackRecord* pRecord, matrix3x4_t* pMatrix, float flSimTime) -> void
		{
			const auto pModel = pPlayer->GetModel();
			if (!pModel) return;

			const auto pHdr = I::ModelInfoClient->GetStudiomodel(pModel);
			if (!pHdr) return;

			const auto pSet = pHdr->pHitboxSet(pPlayer->m_nHitboxSet());
			if (!pSet) return;

			// Determine hitboxes to check based on mode
			int primaryHitbox = 3; // Body
			int secondaryHitbox = 0; // Head
			
			if (Vars::Aimbot::Hitbox == 0)
			{
				primaryHitbox = 0; // Head only
				secondaryHitbox = -1;
			}
			else if (Vars::Aimbot::Hitbox == 1)
			{
				primaryHitbox = 3; // Body only
				secondaryHitbox = -1;
			}
			else if (Vars::Aimbot::Hitbox == 2) // Auto
			{
				if (isRailgunOrSniper)
				{
					primaryHitbox = 0; // Head first for snipers
					secondaryHitbox = 3; // Body fallback
				}
				else
				{
					primaryHitbox = 3; // Body first for others
					secondaryHitbox = 0; // Head fallback
				}
			}

			auto GetHitboxPos = [&](int nIdx) -> Vector {
				if (pRecord) {
					for (const auto& hb : pRecord->Hitboxes) {
						if (hb.nHitboxIndex == nIdx) return hb.vPos;
					}
					return Vector(0,0,0);
				} else {
					Vector vOut;
					if (GetHitbox(pPlayer, vOut, pMatrix)) return vOut;
					return Vector(0,0,0);
				}
			};

			// Check primary hitbox
			Vector vPrimaryHitbox = GetHitboxPos(primaryHitbox);
			bool bPrimaryValid = !vPrimaryHitbox.IsZero();
			
			if (bPrimaryValid)
			{
				bPrimaryValid = IsVisible(pLocal, pPlayer, vPrimaryHitbox);
			}

			// Check secondary hitbox if needed
			Vector vSecondaryHitbox;
			bool bSecondaryValid = false;
			if (!bPrimaryValid && secondaryHitbox >= 0)
			{
				vSecondaryHitbox = GetHitboxPos(secondaryHitbox);
				if (!vSecondaryHitbox.IsZero())
				{
					bSecondaryValid = IsVisible(pLocal, pPlayer, vSecondaryHitbox);
				}
			}

			// Use whichever hitbox is visible
			Vector vFinalHitbox;
			if (bPrimaryValid)
				vFinalHitbox = vPrimaryHitbox;
			else if (bSecondaryValid)
				vFinalHitbox = vSecondaryHitbox;
			else
				return; // No visible hitbox

			const float flFOV = U::Math.GetFovBetween(pCmd->viewangles, U::Math.GetAngleToPosition(vLocalPos, vFinalHitbox));
			
			// Early FOV check for FOV-based targeting
			if (Vars::Aimbot::TargetSelection == 1 && flFOV > Vars::Aimbot::FOV)
				return;

			const float flDistance = vLocalPos.DistTo(vFinalHitbox);

			bool bBetter = false;
			switch (Vars::Aimbot::TargetSelection)
			{
			case 0: // Distance
				bBetter = (bestTarget.pEntity == nullptr || flDistance < bestTarget.flDistance);
				break;
			case 1: // FOV
				bBetter = (bestTarget.pEntity == nullptr || flFOV < bestTarget.flFOV);
				break;
			}

			if (bBetter)
			{
				bestTarget.pEntity = pPlayer;
				bestTarget.vPos = vFinalHitbox;
				bestTarget.flFOV = flFOV;
				bestTarget.flDistance = flDistance;
				bestTarget.flSimTime = flSimTime;
				bestTarget.pBoneMatrix = pMatrix;
			}
		};

		// 1. Check current tick
		matrix3x4_t BoneMatrix[128];
		if (pPlayer->SetupBones(BoneMatrix, 128, 0x100, I::GlobalVarsBase->curtime))
		{
			CheckTarget(nullptr, BoneMatrix, -1.0f);
		}

		// 2. Check Backtrack records
		if (Vars::Backtrack::Enabled)
		{
			const auto* records = F::Backtrack.GetRecords(pPlayer->entindex());
			if (records)
			{
				for (const auto& record : *records)
				{
					// Check validity (185ms) - reuse logic from Backtrack class
					if (!F::Backtrack.IsTickValid(record.flSimulationTime, pPlayer->m_flSimulationTime())) continue;
					
					// Use record matrix
					CheckTarget(&record, const_cast<matrix3x4_t*>(record.BoneMatrix), record.flSimulationTime);
				}
			}
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
	case 0: // Plain
		pCmd->viewangles = vAngle;
		U::Math.ClampAngles(pCmd->viewangles);
		I::EngineClient->SetViewAngles(pCmd->viewangles);
		break;

	case 1: // Smooth
	{
		Vector vDelta = vAngle - pCmd->viewangles;
		U::Math.ClampAngles(vDelta);
		pCmd->viewangles += vDelta / Vars::Aimbot::SmoothAmount;
		U::Math.ClampAngles(pCmd->viewangles);
		I::EngineClient->SetViewAngles(pCmd->viewangles);
		break;
	}

	case 2: // Silent (no PSilent in TF2, it's the same)
	{
		// Only aim when actually attacking
		if (pCmd->buttons & IN_ATTACK)
		{
			U::Math.FixMovement(pCmd, vOldAngles, vAngle);
			pCmd->viewangles = vAngle;
			U::Math.ClampAngles(pCmd->viewangles);
			G::bSilentAngles = true;
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

	if (target.flSimTime > 0.0f)
	{
		float flLerp = F::Backtrack.GetLerp();
		pCmd->tick_count = TIME_TO_TICKS(target.flSimTime) + TIME_TO_TICKS(flLerp);
	}

	AimAt(pLocal, pCmd, target.vPos);
}
