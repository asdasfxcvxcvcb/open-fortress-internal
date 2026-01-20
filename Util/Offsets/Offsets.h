#pragma once

#include "../Pattern/Pattern.h"

class CUtil_Offsets
{
public:
	void Initialize();

public:
	uintptr_t m_dwClientModeCreateMove = 0x0;
	uintptr_t m_dwStartDrawing = 0x0;
	uintptr_t m_dwFinishDrawing = 0x0;
	uintptr_t m_dwIInput_GetUserCmd = 0x0;
	uintptr_t m_dwIInput = 0x0;
	uintptr_t m_dwGlobalVars = 0x0;
	uintptr_t m_dwCNetChan_Shutdown = 0x0;
	uintptr_t m_dwC_BasePlayer_CalcPlayerView = 0x0;
	uintptr_t m_dwCNetChan_SendNetMsg = 0x0;
	uintptr_t m_dwClientState = 0x0;
	uintptr_t m_dwC_BaseEntity_SetPredictionRandomSeed = 0x0;
	uintptr_t m_dwC_BaseEntity_Instance = 0x0;
	uintptr_t m_dwC_BasePlayer_UpdateButtonState = 0x0;
	uintptr_t m_dwC_BasePlayer_UsingStandardWeaponsInVehicle = 0x0;
	uintptr_t m_dwC_BaseCombatWeapon_GetWpnData = 0x0;
	uintptr_t m_dwC_TFPlayerShared_InCondInvis = 0x0;
	uintptr_t m_dwC_TFPlayerShared_InCondUber = 0x0;
	uintptr_t m_dwC_TFPlayerShared_InCond = 0x0;
	uintptr_t m_dwSharedRandomInt = 0x0;
	uintptr_t m_dwCTFGameMovement_ProcessMovement = 0x0;
	uintptr_t m_dwC_TFPlayer_TeamFortress_SetSpeed = 0x0;
	uintptr_t m_dwClientModeShared = 0x0;
	uintptr_t m_dwClientModeShared_OverrideView = 0x0;
};

namespace U { inline CUtil_Offsets Offsets; }