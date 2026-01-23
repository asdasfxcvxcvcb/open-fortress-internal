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

class CBacktrack
{
public:
	void Update();
	void Run(CUserCmd* pCmd);
	bool IsTickValid(float flSimTime, float flPlayerSimTime);
	const std::deque<BacktrackRecord>* GetRecords(C_TFPlayer* pPlayer);
	float GetLerp();
	void DebugDraw();

	// Helpers from example
	float GetReal(int iFlow = 2, bool bNoFake = false); // 2 = MAX_FLOWS
	float GetWishFake();
	float GetWishLerp();
	float GetFakeLatency();
	float GetFakeInterp();
	float GetWindow();
	int GetAnticipatedChoke();

	void AdjustPing(CNetChannel* pNetChan);
	void RestorePing(CNetChannel* pNetChan);

private:
	std::map<C_TFPlayer*, std::deque<BacktrackRecord>> m_Records;
	std::vector<std::string> m_DebugStrings;
	int m_UpdateCount = 0;

	// For ping manipulation
	float m_flFakeLatency = 0.f;
	float m_flSentInterp = 0.f;
	float m_flMaxUnlag = 1.0f;
	int m_nOldInSequenceNr = 0;
	int m_nOldInReliableState = 0;
	int m_nOldTickBase = 0;
	int m_nLastInSequenceNr = 0;
	
	struct SequenceData
	{
		int m_nInReliableState;
		int m_nSequenceNr;
		float m_flTime;

		SequenceData(int instate, int seqnr, float time) 
			: m_nInReliableState(instate), m_nSequenceNr(seqnr), m_flTime(time) {}
	};
	std::deque<SequenceData> m_dSequences;
	int m_iLastInSequence = 0;
};

namespace F { inline CBacktrack Backtrack; }
