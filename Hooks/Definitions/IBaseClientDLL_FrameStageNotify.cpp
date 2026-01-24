#include "../../Hooks/Hooks.h"
#include "../../Features/Backtrack/Backtrack.h"

DEFINE_HOOK(IBaseClientDLL_FrameStageNotify, void, __fastcall, void* ecx, void* edx, ClientFrameStage_t curStage)
{
	Func.Original<FN>()(ecx, edx, curStage);

	if (curStage == FRAME_NET_UPDATE_END)
	{
		F::Backtrack.Update();
	}
}
