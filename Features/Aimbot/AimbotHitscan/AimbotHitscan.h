#pragma once

#include "../../../SDK/SDK.h"
#include "../../Vars.h"

struct AimbotTarget
{
	C_TFPlayer* pEntity = nullptr;
	Vector vPos = {};
	float flFOV = 0.0f;
	float flDistance = 0.0f;
	int nPriority = 0;
};

class CAimbotHitscan
{
private:
	bool GetHitbox(C_TFPlayer* pEntity, Vector& vOut);
	bool IsValidTarget(C_TFPlayer* pLocal, C_TFPlayer* pEntity);
	AimbotTarget GetBestTarget(C_TFPlayer* pLocal, CUserCmd* pCmd);
	void AimAt(C_TFPlayer* pLocal, CUserCmd* pCmd, const Vector& vTarget);
	bool IsVisible(C_TFPlayer* pLocal, C_TFPlayer* pEntity, const Vector& vPos);

public:
	void Run(C_TFPlayer* pLocal, CUserCmd* pCmd);
};

namespace F { inline CAimbotHitscan AimbotHitscan; }
