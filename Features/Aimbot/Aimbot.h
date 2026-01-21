#pragma once

#include "../../SDK/SDK.h"
#include "../Vars.h"

// Forward declarations
class CAimbotHitscan;
class CAimbotProjectile;
class CAimbotMelee;

enum class EWeaponType
{
	HITSCAN,
	PROJECTILE,
	MELEE,
	UNKNOWN
};

class CAimbot
{
private:
	EWeaponType GetWeaponType(C_BaseCombatWeapon* pWeapon);
	bool IsHitscanWeapon(const char* weaponName);
	bool IsProjectileWeapon(const char* weaponName);
	bool IsMeleeWeapon(const char* weaponName);

public:
	void Run(C_TFPlayer* pLocal, CUserCmd* pCmd);
	void DrawFOV();
	
	// Track if aimbot is currently aiming at a target
	inline bool IsAiming() const { return m_bIsAiming; }
	inline void SetAiming(bool bAiming) { m_bIsAiming = bAiming; }
	
private:
	bool m_bIsAiming = false;
};

namespace F { inline CAimbot Aimbot; }
