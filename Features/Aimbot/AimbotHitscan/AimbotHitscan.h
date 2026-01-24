#pragma once

#include "../../../SDK/SDK.h"
#include "../../Vars.h"

struct AimbotTarget
{
	C_TFPlayer* pEntity = nullptr;
	Vector vPos = {};
	float flFOV = 0.0f;
	float flDistance = 0.0f;
	float flSimTime = -1.0f;
	matrix3x4_t* pBoneMatrix = nullptr;
};

class CAimbotHitscan
{
private:
	bool GetHitbox(C_TFPlayer* pEntity, Vector& vOut, matrix3x4_t* pBoneMatrix = nullptr);
	bool IsVisible(C_TFPlayer* pLocal, C_TFPlayer* pEntity, const Vector& vPos);
	AimbotTarget GetBestTarget(C_TFPlayer* pLocal, CUserCmd* pCmd);
	void AimAt(C_TFPlayer* pLocal, CUserCmd* pCmd, const Vector& vTarget);

public:
	void Run(C_TFPlayer* pLocal, CUserCmd* pCmd);
};

namespace F { inline CAimbotHitscan AimbotHitscan; }
