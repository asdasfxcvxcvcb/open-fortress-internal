#include "Backtrack.h"
#include "../Vars.h"
#include "../../SDK/Globals.h"
#include "../../Util/Math/Math.h"
#include "../../SDK/Interfaces/IVEngineClient.h"
#include "../../SDK/Includes/client_class.h"
#include <algorithm>
#include <string>

// Minimal interface for GetLatency
class INetChannelInfo
{
public:
	virtual const char* GetName(void) const = 0;
	virtual const char* GetAddress(void) const = 0;
	virtual float GetTime(void) const = 0;
	virtual float GetTimeConnected(void) const = 0;
	virtual int GetBufferSize(void) const = 0;
	virtual int GetDataRate(void) const = 0;
	virtual bool IsLoopback(void) const = 0;
	virtual bool IsTimingOut(void) const = 0;
	virtual bool IsPlayback(void) const = 0;
	virtual float GetLatency(int flow) const = 0;
	virtual float GetAvgLatency(int flow) const = 0;
	virtual float GetAvgLoss(int flow) const = 0;
	virtual float GetAvgChoke(int flow) const = 0;
	virtual float GetAvgData(int flow) const = 0;
	virtual float GetAvgPackets(int flow) const = 0;
	virtual int GetTotalData(int flow) const = 0;
	virtual int GetSequenceNr(int flow) const = 0;
	virtual bool IsValidPacket(int flow, int frame_number) const = 0;
	virtual float GetPacketTime(int flow, int frame_number) const = 0;
	virtual int GetPacketBytes(int flow, int frame_number, int group) const = 0;
	virtual bool GetStreamProgress(int flow, int* received, int* total) const = 0;
	virtual float GetTimeSinceLastReceived(void) const = 0;
	virtual float GetCommandInterpolationAmount(int flow, int frame_number) const = 0;
	virtual void GetPacketResponseLatency(int flow, int frame_number, int* pnLatencyMsecs, int* pnChoke) const = 0;
	virtual void GetRemoteFramerate(float* pflFrameTime, float* pflFrameTimeStdDeviation) const = 0;
	virtual float GetTimeoutSeconds(void) const = 0;
};

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

	if (!Vars::Backtrack::Enabled)
	{
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

	// Track active indices to clean up disconnected players
	static std::vector<int> activeIndices;
	activeIndices.clear();

	for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++)
	{
		if (i == nLocalIndex)
			continue;

		C_BaseEntity* pEntity = I::ClientEntityList->GetClientEntity(i)->As<C_BaseEntity*>();
		if (!pEntity || pEntity->IsDormant() || !pEntity->IsPlayer())
		{
			continue;
		}

		activeIndices.push_back(i);

		C_TFPlayer* pPlayer = pEntity->As<C_TFPlayer*>();
		if (!pPlayer->IsAlive())
		{
			m_Records[i].clear();
			continue;
		}

		if (!Vars::Aimbot::FFAMode && pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
		{
			m_Records[i].clear();
			continue;
		}

		// Create record
		BacktrackRecord record;
		record.flSimulationTime = pPlayer->m_flSimulationTime();
		
		// Skip if SimTime is invalid or didn't change (unless it's the first record)
		if (record.flSimulationTime == 0.0f)
			continue;
			
		record.nTickCount = TIME_TO_TICKS(record.flSimulationTime);
		record.vOrigin = pPlayer->m_vecOrigin();
		record.vAbsOrigin = pPlayer->GetAbsOrigin();
		record.vAbsAngles = pPlayer->GetAbsAngles();
		
		// Use current time for bones to match what the user sees (interpolated)
		// We do NOT disable interpolation (EF_NOINTERP) because we want to hit what we see.
		if (!pPlayer->GetModel() || !pPlayer->SetupBones(record.BoneMatrix, 128, BONE_USED_BY_HITBOX, record.flSimulationTime))
		{
			continue;
		}

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

		if (m_Records[i].empty() || record.flSimulationTime > m_Records[i].front().flSimulationTime)
		{
			m_Records[i].push_front(record);
		}

		// Cleanup old records
		while (!m_Records[i].empty())
		{
			auto& last = m_Records[i].back();
			auto& first = m_Records[i].front();
			
			// Enforce Window Size strictly based on HISTORY LENGTH (First SimTime - Last SimTime)
			// This ensures we keep X ms of history regardless of how high cl_interp is.
			float flHistoryLength = first.flSimulationTime - last.flSimulationTime;
			float flWindow = GetWindow();

			if (flHistoryLength > flWindow)
			{
				m_Records[i].pop_back();
				continue;
			}
			
			// Also enforce hard server limits (0.2s / sv_maxunlag) using standard IsTickValid
			// But do NOT use our custom Window in IsTickValid, use the hard limit there.
			if (!IsTickValid(last.flSimulationTime, I::GlobalVarsBase->curtime))
			{
				m_Records[i].pop_back();
			}
			else
			{
				break;
			}
		}
	}

	// Remove records for players that are no longer valid (disconnected or dormant)
	for (auto it = m_Records.begin(); it != m_Records.end();)
	{
		bool bFound = false;
		for (int idx : activeIndices)
		{
			if (idx == it->first)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
			it = m_Records.erase(it);
		else
			++it;
	}
}

void CBacktrack::Run(CUserCmd* pCmd)
{
	if (!Vars::Backtrack::Enabled)
		return;

	// Don't interfere if Aimbot is already active
	if (G::bAimbotActive)
		return;

	if (!(pCmd->buttons & IN_ATTACK))
		return;

	C_TFPlayer* pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->As<C_TFPlayer*>();
	if (!pLocal) return;

	// Skip backtracking for projectile weapons
	C_BaseCombatWeapon* pWeapon = pLocal->GetActiveWeapon();
	if (pWeapon)
	{
		const char* weaponName = pWeapon->GetName();
		if (weaponName)
		{
			std::string weaponStr = weaponName;
			std::transform(weaponStr.begin(), weaponStr.end(), weaponStr.begin(), ::tolower);

			bool isProjectile = (weaponStr.find("rocket") != std::string::npos ||
								 weaponStr.find("grenade") != std::string::npos ||
								 weaponStr.find("pipe") != std::string::npos ||
								 weaponStr.find("nailgun") != std::string::npos ||
								 weaponStr.find("syringe") != std::string::npos ||
								 weaponStr.find("tranq") != std::string::npos ||
								 weaponStr.find("flare") != std::string::npos ||
								 weaponStr.find("arrow") != std::string::npos);

			if (isProjectile)
				return;
		}
	}

	Vector vViewAngles = pCmd->viewangles;
	Vector vEyePos = pLocal->GetShootPos();

	float flBestFOV = 255.0f;
	float flBestSimTime = -1.0f;

	// Iterate safely using integer keys
	for (auto& [iEntityIndex, records] : m_Records)
	{
		C_BaseEntity* pEntity = I::ClientEntityList->GetClientEntity(iEntityIndex)->As<C_BaseEntity*>();
		if (!pEntity || pEntity->IsDormant()) continue;
		
		C_TFPlayer* pPlayer = pEntity->As<C_TFPlayer*>();
		if (!pPlayer->IsAlive()) continue;

		for (const auto& record : records)
		{
			if (!IsTickValid(record.flSimulationTime, I::GlobalVarsBase->curtime))
				continue;

			// Check hitboxes
			for (const auto& hb : record.Hitboxes)
			{
				float fov = U::Math.GetFovBetween(vViewAngles, U::Math.GetAngleToPosition(vEyePos, hb.vPos));
				
				// Prefer newer records (we iterate Newest -> Oldest)
				// If we find a newer record with similar FOV (within 1 degree), keep it (or take it if we found it later, but we iterate Newest first so we already have it)
				// Actually, since we iterate Newest -> Oldest:
				// 1. Found Newest (Sim=100). Best=100.
				// 2. Found Old (Sim=90).
				// We want to KEEP Newest unless Old is strictly better.
				// So we only update if fov is BETTER.
				// The previous logic was: if (fov < Best + 1.0 && Sim < BestSim). This explicitly SWAPPED to Old.
				// We change to: Only swap if fov is strictly better.
				// Or if fov is similar, prefer Newer. (Which we already have).
				
				if (flBestSimTime == -1.0f || fov < flBestFOV)
				{
					flBestFOV = fov;
					flBestSimTime = record.flSimulationTime;
				}
			}
		}
	}

	if (flBestSimTime != -1.0f && flBestFOV < 5.0f)
	{
		// Set tick count to the exact simulation time of the record (No Lerp)
		pCmd->tick_count = TIME_TO_TICKS(flBestSimTime);
	}
}

bool CBacktrack::IsTickValid(float flSimTime, float flCurTime)
{
	float flCorrect = std::clamp(GetReal(MAX_FLOWS, false) + GetLerp(), 0.f, m_flMaxUnlag);
	float flDelta = fabsf(flCorrect - (flCurTime - flSimTime));
	
	// Server limit is usually 200ms, but we use 250ms for safety/rounding/stability
	return flDelta < 0.25f;
}

const std::deque<BacktrackRecord>* CBacktrack::GetRecords(int iEntityIndex)
{
	if (m_Records.find(iEntityIndex) == m_Records.end())
		return nullptr;
	return &m_Records[iEntityIndex];
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
	INetChannelInfo* pNetChan = reinterpret_cast<INetChannelInfo*>(I::EngineClient->GetNetChannelInfo());
	if (!pNetChan)
		return 0.f;

	if (iFlow != MAX_FLOWS)
		return pNetChan->GetLatency(iFlow);
		
	return pNetChan->GetLatency(FLOW_INCOMING) + pNetChan->GetLatency(FLOW_OUTGOING);
}

float CBacktrack::GetWindow()
{
	return Vars::Backtrack::flBacktrackWindowSize / 1000.f;
}

void CBacktrack::DebugDraw()
{
	// Skeleton rendering is handled by ESP::RenderLagRecords
}