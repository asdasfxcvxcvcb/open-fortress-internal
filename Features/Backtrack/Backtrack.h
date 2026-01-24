#pragma once

#include "../../SDK/SDK.h"
#include "../../SDK/Entities/C_TFPlayer.h"
#include <deque>
#include <map>

// Backtrack record structure
struct BacktrackRecord
{
	float flSimulationTime = 0.0f;
	int nTickCount = 0;
	Vector vOrigin = {};
	Vector vAbsOrigin = {};
	Vector vAbsAngles = {};
	matrix3x4_t BoneMatrix[128] = {};
	
	// Pre-transformed hitbox positions to avoid needing SetupBones during visibility check
	struct HitboxData
	{
		Vector vPos;
		Vector vMin;
		Vector vMax;
		int nBone;
		int nHitboxIndex;
	};
	std::vector<HitboxData> Hitboxes;
};

class C_TFPlayer;
class CUserCmd;

class CBacktrack
{
public:
	void Update();
	void Run(CUserCmd* pCmd);
	bool IsTickValid(float flSimTime, float flPlayerSimTime);
	const std::deque<BacktrackRecord>* GetRecords(int iEntityIndex);
	float GetLerp();
	void DebugDraw();

	// Helpers
	float GetReal(int iFlow = 2, bool bNoFake = false); // 2 = MAX_FLOWS
	float GetWindow();

private:
	std::map<int, std::deque<BacktrackRecord>> m_Records;
	std::vector<std::string> m_DebugStrings;
	int m_UpdateCount = 0;

	float m_flMaxUnlag = 1.0f;
};

namespace F { inline CBacktrack Backtrack; }
