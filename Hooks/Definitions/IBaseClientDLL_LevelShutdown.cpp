#include "../../Hooks/Hooks.h"

DEFINE_HOOK(IBaseClientDLL_LevelShutdown, void, __fastcall, void* ecx, void* edx)
{
	// Reset global states on level change
	G::bSilentAngles = false;
	G::bPSilentAngles = false;
	
	Func.Original<FN>()(ecx, edx);
}