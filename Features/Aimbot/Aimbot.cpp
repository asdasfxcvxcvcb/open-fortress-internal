#include "Aimbot.h"
#include "AimbotHitscan/AimbotHitscan.h"
#include "AimbotProjectile/AimbotProjectile.h"
#include "AimbotMelee/AimbotMelee.h"
#include <string>
#include <algorithm>

bool CAimbot::IsHitscanWeapon(const char* weaponName)
{
	if (!weaponName)
		return false;

	std::string weaponStr = weaponName;
	std::transform(weaponStr.begin(), weaponStr.end(), weaponStr.begin(), ::tolower);

	return (weaponStr.find("shotgun") != std::string::npos ||
			weaponStr.find("scattergun") != std::string::npos ||
			weaponStr.find("supershotgun") != std::string::npos ||
			weaponStr.find("smg") != std::string::npos ||
			weaponStr.find("tommygun") != std::string::npos ||
			weaponStr.find("assaultrifle") != std::string::npos ||
			weaponStr.find("minigun") != std::string::npos ||
			weaponStr.find("gatlinggun") != std::string::npos ||
			weaponStr.find("pistol") != std::string::npos ||
			weaponStr.find("revolver") != std::string::npos ||
			weaponStr.find("sniperrifle") != std::string::npos ||
			weaponStr.find("railgun") != std::string::npos ||
			weaponStr.find("lightning") != std::string::npos);
}

bool CAimbot::IsProjectileWeapon(const char* weaponName)
{
	if (!weaponName)
		return false;

	std::string weaponStr = weaponName;
	std::transform(weaponStr.begin(), weaponStr.end(), weaponStr.begin(), ::tolower);

	return (weaponStr.find("rocket") != std::string::npos ||
			weaponStr.find("grenade") != std::string::npos ||
			weaponStr.find("pipe") != std::string::npos ||
			weaponStr.find("nailgun") != std::string::npos ||
			weaponStr.find("syringe") != std::string::npos ||
			weaponStr.find("tranq") != std::string::npos ||
			weaponStr.find("flare") != std::string::npos ||
			weaponStr.find("arrow") != std::string::npos);
}

bool CAimbot::IsMeleeWeapon(const char* weaponName)
{
	if (!weaponName)
		return false;

	std::string weaponStr = weaponName;
	std::transform(weaponStr.begin(), weaponStr.end(), weaponStr.begin(), ::tolower);

	return (weaponStr.find("crowbar") != std::string::npos ||
			weaponStr.find("knife") != std::string::npos ||
			weaponStr.find("bat") != std::string::npos ||
			weaponStr.find("bottle") != std::string::npos ||
			weaponStr.find("axe") != std::string::npos ||
			weaponStr.find("club") != std::string::npos ||
			weaponStr.find("fists") != std::string::npos ||
			weaponStr.find("shovel") != std::string::npos ||
			weaponStr.find("wrench") != std::string::npos ||
			weaponStr.find("bonesaw") != std::string::npos ||
			weaponStr.find("chainsaw") != std::string::npos ||
			weaponStr.find("claws") != std::string::npos ||
			weaponStr.find("combatknife") != std::string::npos ||
			weaponStr.find("pipe") != std::string::npos && weaponStr.find("lead") != std::string::npos);
}

EWeaponType CAimbot::GetWeaponType(C_BaseCombatWeapon* pWeapon)
{
	if (!pWeapon)
		return EWeaponType::UNKNOWN;

	const char* weaponName = pWeapon->GetName();
	if (!weaponName)
		return EWeaponType::UNKNOWN;

	if (IsHitscanWeapon(weaponName))
		return EWeaponType::HITSCAN;
	
	if (IsProjectileWeapon(weaponName))
		return EWeaponType::PROJECTILE;
	
	if (IsMeleeWeapon(weaponName))
		return EWeaponType::MELEE;

	return EWeaponType::UNKNOWN;
}

bool CAimbot::IsValidTarget(C_TFPlayer* pLocal, C_TFPlayer* pEntity)
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
		if (pEntity->InCondUber())
			return false;
	}

	return true;
}

void CAimbot::CalculateFOVRadius(C_TFPlayer* pLocal)
{
	if (!pLocal) return;

	float viewFov = static_cast<float>(pLocal->m_iFOV());
	if (viewFov == 0.0f) viewFov = static_cast<float>(pLocal->m_iDefaultFOV());
	if (viewFov == 0.0f) viewFov = 90.0f;

	float viewFovRadians = (viewFov * 3.14159f) / 180.0f;
	float aimFovRadians = (Vars::Aimbot::FOV * 3.14159f) / 180.0f;

	// Calculate radius using Screen Width (Robust method)
	// Radius = tan(AimFOV) / tan(ViewFOV / 2) * (ScreenWidth / 2)
	m_nFOVRadius = static_cast<int>((H::Draw.m_nScreenW / 2.0f) * (tanf(aimFovRadians) / tanf(viewFovRadians / 2.0f)));
}

bool CAimbot::ShouldIgnore(C_TFPlayer* pLocal, CUserCmd* pCmd, const Vector& vTargetPos)
{
	if (!pLocal) return true;

	// Ensure radius is up to date
	CalculateFOVRadius(pLocal);

	// Project target to screen
	Vector2D vScreen;
	if (!H::Draw.WorldPosToScreenPos(vTargetPos, vScreen))
		return true; // Target is off-screen (behind us), so definitely ignore

	// Calculate 2D distance from screen center
	float centerX = H::Draw.m_nScreenW / 2.0f;
	float centerY = H::Draw.m_nScreenH / 2.0f;
	
	float dx = vScreen.x - centerX;
	float dy = vScreen.y - centerY;
	float dist = sqrtf(dx * dx + dy * dy);

	// Check against calculated pixel radius
	if (dist > m_nFOVRadius)
		return true;

	return false;
}

void CAimbot::Run(C_TFPlayer* pLocal, CUserCmd* pCmd)
{
	SetAiming(false);
	G::bAimbotActive = false;
	
	if (!Vars::Aimbot::Enabled)
		return;

	if (!pLocal || !pLocal->IsAlive())
		return;

	// Pre-calculate FOV radius for this frame
	CalculateFOVRadius(pLocal);

	C_BaseCombatWeapon* pWeapon = pLocal->GetActiveWeapon();
	EWeaponType weaponType = GetWeaponType(pWeapon);

	switch (weaponType)
	{
		case EWeaponType::HITSCAN:
			F::AimbotHitscan.Run(pLocal, pCmd);
			break;

		case EWeaponType::PROJECTILE:
			F::AimbotProjectile.Run(pLocal, pCmd);
			break;

		case EWeaponType::MELEE:
			F::AimbotMelee.Run(pLocal, pCmd);
			break;

		case EWeaponType::UNKNOWN:
		default:
			break;
	}
}

void CAimbot::DrawFOV(C_TFPlayer* pLocal)
{
	if (!Vars::Aimbot::Enabled || !Vars::Aimbot::DrawFOV || Vars::Aimbot::TargetSelection != 1)
		return;

	// Ensure updated radius
	CalculateFOVRadius(pLocal);

	int centerX = H::Draw.m_nScreenW / 2;
	int centerY = H::Draw.m_nScreenH / 2;

	H::Draw.OutlinedCircle(centerX, centerY, m_nFOVRadius, 64, Color(255, 255, 255, 100));
}
