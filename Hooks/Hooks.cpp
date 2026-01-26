#include "Hooks.h"

void CGlobal_Hooks::Initialize()
{
	XASSERT(MH_Initialize() != MH_STATUS::MH_OK);

	Hooks::CPrediction_RunCommand::Initialize();
	Hooks::IBaseClientDLL_FrameStageNotify::Initialize();
	Hooks::IBaseClientDLL_LevelShutdown::Initialize();
	Hooks::IBaseClientDLL_LevelInitPostEntity::Initialize();
	Hooks::ClientModeShared_CreateMove::Initialize();
	Hooks::IEngineVGui_Paint::Initialize();
	Hooks::ISurface_LockCursor::Initialize();
	
	// Hook OverrideView using signature
	if (U::Offsets.m_dwClientModeShared_OverrideView)
	{
		Hooks::ClientModeShared_OverrideView::Initialize();
	}

	// Hook CL_Move using signature
	if (U::Offsets.m_dwCL_Move)
	{
		Hooks::CL_Move::Initialize();
	}

	XASSERT(MH_EnableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK);
}