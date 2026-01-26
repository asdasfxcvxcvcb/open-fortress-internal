#pragma once

#include "../SDK/SDK.h"

CREATE_HOOK(ClientModeShared_CreateMove, U::Offsets.m_dwClientModeCreateMove, bool, __fastcall, void* ecx, void* edx, float input_sample_frametime, CUserCmd* cmd);
CREATE_HOOK(ClientModeShared_OverrideView, U::Offsets.m_dwClientModeShared_OverrideView, void, __fastcall, void* ecx, void* edx, CViewSetup* pSetup);
CREATE_HOOK(IEngineVGui_Paint, U::VFunc.Get<void*>(I::EngineVGui, 13u), void, __fastcall, void* ecx, void* edx, int mode);
CREATE_HOOK(IBaseClientDLL_FrameStageNotify, U::VFunc.Get<void*>(I::BaseClientDLL, 35u), void, __fastcall, void* ecx, void* edx, ClientFrameStage_t curStage);
CREATE_HOOK(IBaseClientDLL_LevelShutdown, U::VFunc.Get<void*>(I::BaseClientDLL, 7u), void, __fastcall, void* ecx, void* edx);
CREATE_HOOK(IBaseClientDLL_LevelInitPostEntity, U::VFunc.Get<void*>(I::BaseClientDLL, 6u), void, __fastcall, void* ecx, void* edx);
CREATE_HOOK(CPrediction_RunCommand, U::VFunc.Get<void*>(I::Prediction, 17u), void, __fastcall, void* ecx, void* edx, C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper);
CREATE_HOOK(ISurface_LockCursor, U::VFunc.Get<void*>(I::MatSystemSurface, 62u), void, __fastcall, void* ecx, void* edx);
CREATE_HOOK(CL_Move, U::Offsets.m_dwCL_Move, void, __cdecl, float accumulated_extra_samples, bool bFinalTick);

class CGlobal_Hooks
{
public:
	void Initialize();
};

namespace G { inline CGlobal_Hooks Hooks; }