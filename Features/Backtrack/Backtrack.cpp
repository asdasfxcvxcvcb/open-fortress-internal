#include "Backtrack.h"
#include "../Vars.h"
#include "../../Util/Math/Math.h"
#include <algorithm>
#include <string>

// Helper macros if not defined
#ifndef FLOW_OUTGOING
#define FLOW_OUTGOING 0
#endif
#ifndef FLOW_INCOMING
#define FLOW_INCOMING 1
#endif
#ifndef MAX_FLOWS
#define MAX_FLOWS 2
#endif

void CBacktrack::Update()
{
	m_UpdateCount++;
	m_DebugStrings.clear();

	// Update sequence data for ping manipulation
	CNetChannel* pNetChan = I::EngineClient->GetNetChannelInfo();
	if (pNetChan)
	{
		if (pNetChan->m_nInSequenceNr > m_iLastInSequence)
		{
			m_iLastInSequence = pNetChan->m_nInSequenceNr;
			m_dSequences.emplace_front(pNetChan->m_nInReliableState, pNetChan->m_nInSequenceNr, I::GlobalVars->realtime);
		}

		if (m_dSequences.size() > 67)
			m_dSequences.pop_back();
	}

	if (!Vars::Backtrack::Enabled)
	{
		m_DebugStrings.push_back("Backtrack disabled in menu");
		if (!m_Records.empty())
			m_Records.clear();
		return;
	}

	static auto sv_maxunlag = I::Cvar->FindVar("sv_maxunlag");
	m_flMaxUnlag = sv_maxunlag ? sv_maxunlag->GetFloat() : 1.0f;

	const int nLocalIndex = I::EngineClient->GetLocalPlayer();
	const auto pLocal = I::ClientEntityList->GetClientEntity(nLocalIndex)->As<C_TFPlayer*>();

	if (!pLocal || !pLocal->IsAlive())
	{
		m_Records.clear();
		return;
	}

	for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
	{
		if (i == nLocalIndex)
			continue;

		C_BaseEntity* pEntity = I::ClientEntityList->GetClientEntity(i)->As<C_BaseEntity*>();
		if (!pEntity || pEntity->IsDormant())
		{
			m_Records.erase(reinterpret_cast<C_TFPlayer*>(pEntity));
			continue;
		}

		C_TFPlayer* pPlayer = pEntity->As<C_TFPlayer*>();
		if (!pPlayer->IsAlive())
		{
			m_Records[pPlayer].clear();
			continue;
		}

		if (!Vars::Aimbot::FFAMode && pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
		{
			m_Records[pPlayer].clear();
			continue;
		}

		// Create record
		BacktrackRecord record;
		record.flSimulationTime = pPlayer->m_flSimulationTime();
		record.nTickCount = TIME_TO_TICKS(record.flSimulationTime);
		record.vOrigin = pPlayer->m_vecOrigin();
		record.vAbsOrigin = pPlayer->GetAbsOrigin();
		record.vAbsAngles = pPlayer->GetAbsAngles();
		
		const int fOldEffects = pPlayer->m_fEffects();
		pPlayer->m_fEffects() |= 8; // EF_NOINTERP

		if (!pPlayer->SetupBones(record.BoneMatrix, 128, BONE_USED_BY_HITBOX, pPlayer->m_flSimulationTime()))
		{
			pPlayer->m_fEffects() = fOldEffects;
			continue;
		}
		
		pPlayer->m_fEffects() = fOldEffects;

		const auto pModel = pPlayer->GetModel();
		if (pModel)
		{
			const auto pHdr = I::ModelInfoClient->GetStudiomodel(pModel);
			if (pHdr)
			{
				const auto pSet = pHdr->pHitboxSet(pPlayer->m_nHitboxSet());
				if (pSet)
				{
					for (int h = 0; h < pSet->numhitboxes; h++)
					{
						const auto pBox = pSet->pHitbox(h);
						if (!pBox) continue;

						Vector vMin, vMax;
						U::Math.VectorTransform(pBox->bbmin, record.BoneMatrix[pBox->bone], vMin);
						U::Math.VectorTransform(pBox->bbmax, record.BoneMatrix[pBox->bone], vMax);
						
						BacktrackRecord::HitboxData hb;
						hb.vPos = (vMin + vMax) * 0.5f;
						hb.vMin = vMin;
						hb.vMax = vMax;
						hb.nBone = pBox->bone;
						hb.nHitboxIndex = h;
						record.Hitboxes.push_back(hb);
					}
				}
			}
		}

		if (m_Records[pPlayer].empty() || record.flSimulationTime > m_Records[pPlayer].front().flSimulationTime)
		{
			m_Records[pPlayer].push_front(record);
		}

		// Cleanup old records
		while (!m_Records[pPlayer].empty())
		{
			auto& last = m_Records[pPlayer].back();
			if (!IsTickValid(last.flSimulationTime, I::GlobalVars->curtime)) // Check against curtime, logic inside IsTickValid handles the rest
				m_Records[pPlayer].pop_back();
			else
				break;
		}
	}
}

void CBacktrack::Run(CUserCmd* pCmd)
{
	if (!Vars::Backtrack::Enabled)
		return;

	if (!(pCmd->buttons & IN_ATTACK))
		return;

	// Simple check: if aimbot is running and has a target, we assume it set the tick count.
	// But if aimbot is "Legit" or disabled, we need to handle manual shots.
	// For now, always try to find a backtracking target if one hasn't been set?
	// The safest bet is: if we find a record under crosshair, use it.

	C_TFPlayer* pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->As<C_TFPlayer*>();
	if (!pLocal) return;

	Vector vViewAngles = pCmd->viewangles;
	Vector vEyePos = pLocal->GetEyePosition();

	float flBestFOV = 255.0f;
	int nBestTick = -1;

	for (auto& [player, records] : m_Records)
	{
		if (!player || player->IsDormant() || !player->IsAlive()) continue;

		for (const auto& record : records)
		{
			if (!IsTickValid(record.flSimulationTime, I::GlobalVars->curtime))
				continue;

			// Check hitboxes
			for (const auto& hb : record.Hitboxes)
			{
				float fov = U::Math.GetFovBetween(vViewAngles, U::Math.GetAngleToPosition(vEyePos, hb.vPos));
				if (fov < flBestFOV)
				{
					flBestFOV = fov;
					nBestTick = record.nTickCount;
				}
			}
		}
	}

	// If we found a record very close to crosshair, backtrack to it
	if (nBestTick != -1 && flBestFOV < 5.0f) // 5 degrees tolerance
	{
		pCmd->tick_count = nBestTick;
	}
}

bool CBacktrack::IsTickValid(float flSimTime, float flCurTime)
{
	CNetChannel* pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pNetChan)
		return false;

	float flCorrect = std::clamp(GetReal(MAX_FLOWS, false) + GetFakeInterp(), 0.f, m_flMaxUnlag);
	float flDelta = fabsf(flCorrect - (flCurTime - flSimTime));
	
	return flDelta < GetWindow();
}

const std::deque<BacktrackRecord>* CBacktrack::GetRecords(C_TFPlayer* pPlayer)
{
	if (m_Records.find(pPlayer) == m_Records.end())
		return nullptr;
	return &m_Records[pPlayer];
}

float CBacktrack::GetLerp()
{
	static auto cl_interp = I::Cvar->FindVar("cl_interp");
	static auto cl_interp_ratio = I::Cvar->FindVar("cl_interp_ratio");
	static auto cl_updaterate = I::Cvar->FindVar("cl_updaterate");

	float lerp = cl_interp ? cl_interp->GetFloat() : 0.1f;
	float ratio = cl_interp_ratio ? cl_interp_ratio->GetFloat() : 2.0f;
	float rate = cl_updaterate ? cl_updaterate->GetFloat() : 66.0f;

	if (ratio / rate > lerp)
		lerp = ratio / rate;

	return lerp;
}

float CBacktrack::GetReal(int iFlow, bool bNoFake)
{
	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pNetChan)
		return 0.f;

	if (iFlow != MAX_FLOWS)
		return pNetChan->GetLatency(iFlow) - (bNoFake && iFlow == FLOW_INCOMING ? GetFakeLatency() : 0.f);
	return pNetChan->GetLatency(FLOW_INCOMING) + pNetChan->GetLatency(FLOW_OUTGOING) - (bNoFake ? GetFakeLatency() : 0.f);
}

float CBacktrack::GetWishFake()
{
	return std::clamp(Vars::Backtrack::Latency.Value / 1000.f, 0.f, m_flMaxUnlag);
}

float CBacktrack::GetWishLerp()
{
	return std::clamp(Vars::Backtrack::Interp.Value / 1000.f, GetLerp(), m_flMaxUnlag);
}

float CBacktrack::GetFakeLatency()
{
	return m_flFakeLatency;
}

float CBacktrack::GetFakeInterp()
{
	return m_flFakeInterp;
}

float CBacktrack::GetWindow()
{
	return Vars::Backtrack::Window.Value / 1000.f;
}

int CBacktrack::GetAnticipatedChoke()
{
	// Simplified version
	return 0;
}

void CBacktrack::AdjustPing(CNetChannel* pNetChan)
{
	m_nOldInSequenceNr = pNetChan->m_nInSequenceNr;
	m_nOldInReliableState = pNetChan->m_nInReliableState;

	if (!Vars::Backtrack::Latency.Value)
		return;

	auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal)
		return;
	
	// Assuming 1.0f timescale
	float flTimescale = 1.0f; 

	static float flStaticReal = 0.f;
	float flFake = GetWishFake();
	// Approximating tickbase delta... simplifed for now
	float flReal = GetReal(MAX_FLOWS, true); 

	flStaticReal += (flReal + 5 * TICK_INTERVAL - flStaticReal) * 0.1f;

	int nInReliableState = pNetChan->m_nInReliableState;
	int nInSequenceNr = pNetChan->m_nInSequenceNr;
	float flLatency = 0.f;

	for (auto& cSequence : m_dSequences)
	{
		nInReliableState = cSequence.m_nInReliableState;
		nInSequenceNr = cSequence.m_nSequenceNr;
		flLatency = (I::GlobalVars->realtime - cSequence.m_flTime) * flTimescale - TICK_INTERVAL;

		if (flLatency > flFake || m_nLastInSequenceNr >= cSequence.m_nSequenceNr || flLatency > m_flMaxUnlag - flStaticReal)
			break;
	}
	
	pNetChan->m_nInReliableState = nInReliableState;
	pNetChan->m_nInSequenceNr = nInSequenceNr;

	m_nLastInSequenceNr = pNetChan->m_nInSequenceNr;
}

void CBacktrack::RestorePing(CNetChannel* pNetChan)
{
	pNetChan->m_nInSequenceNr = m_nOldInSequenceNr;
	pNetChan->m_nInReliableState = m_nOldInReliableState;
}

void CBacktrack::DebugDraw()
{
	if (!Vars::Backtrack::Enabled || !I::EngineClient->IsInGame())
		return;

	int y = 300;
	H::Draw.String(EFonts::DEBUG, 10, y, { 255, 255, 255, 255 }, TXT_DEFAULT, "Backtrack DEBUG"); y += 15;
	H::Draw.String(EFonts::DEBUG, 10, y, { 255, 255, 255, 255 }, TXT_DEFAULT, "Updates: %d | Window: %.3f", m_UpdateCount, GetWindow()); y += 15;

	for (const auto& [player, records] : m_Records)
	{
		if (!player || player->IsDormant() || !player->IsAlive()) continue;
		
		if (!records.empty())
		{
			const auto& last = records.back();
			bool valid = IsTickValid(last.flSimulationTime, I::GlobalVars->curtime);
			float delta = I::GlobalVars->curtime - last.flSimulationTime;
			H::Draw.String(EFonts::DEBUG, 20, y, valid ? Color(0, 255, 0, 255) : Color(255, 0, 0, 255), TXT_DEFAULT, 
				"Player %d: %d records | Age: %.3f", player->entindex(), records.size(), delta); 
			y += 15;
		}
	}
}