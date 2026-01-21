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

void CAimbot::Run(C_TFPlayer* pLocal, CUserCmd* pCmd)
{
	SetAiming(false);
	
	if (!Vars::Aimbot::Enabled)
		return;

	if (!pLocal || !pLocal->IsAlive())
		return;

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

void CAimbot::DrawFOV()
{
	if (!Vars::Aimbot::Enabled || !Vars::Aimbot::DrawFOV || Vars::Aimbot::TargetSelection != 1)
		return;

	int centerX = H::Draw.m_nScreenW / 2;
	int centerY = H::Draw.m_nScreenH / 2;

	float fovRadians = (Vars::Aimbot::FOV * 3.14159f) / 180.0f;
	int radius = static_cast<int>((H::Draw.m_nScreenH / 2.0f) * tanf(fovRadians / 2.0f));

	H::Draw.OutlinedCircle(centerX, centerY, radius, 64, Color(255, 255, 255, 100));
}
