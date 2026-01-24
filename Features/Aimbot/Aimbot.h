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
	void DrawFOV(C_TFPlayer* pLocal);
	
	// Shared target validation for all aimbot types
	bool IsValidTarget(C_TFPlayer* pLocal, C_TFPlayer* pEntity);
	
	// Centralized FOV check (Screen-Space)
	bool ShouldIgnore(C_TFPlayer* pLocal, CUserCmd* pCmd, const Vector& vTargetPos);
	
	// Track if aimbot is currently aiming at a target
	inline bool IsAiming() const { return m_bIsAiming; }
	inline void SetAiming(bool bAiming) { m_bIsAiming = bAiming; }

	// Helper to get current radius
	int GetFOVRadius() const { return m_nFOVRadius; }
	
private:
	void CalculateFOVRadius(C_TFPlayer* pLocal);

	bool m_bIsAiming = false;
	int m_nFOVRadius = 0;
};

namespace F { inline CAimbot Aimbot; }
