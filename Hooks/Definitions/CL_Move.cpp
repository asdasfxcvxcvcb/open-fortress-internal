#include "../Hooks.h"
#include "../../Features/SequenceFreezing/SequenceFreezing.h"
#include "../../SDK/Interfaces/IVEngineClient.h"
#include "../../SDK/Entities/C_BasePlayer.h"

DEFINE_HOOK(CL_Move, void, __cdecl, float accumulated_extra_samples, bool bFinalTick)
{
	// Sequence freezing needs to happen here, before packets are sent
	if (bFinalTick)
	{
		auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->As<C_BasePlayer*>();
		if (pLocal)
		{
			F::SequenceFreezing.Run(pLocal, nullptr);
		}
	}

	// Call original
	Func.Original<FN>()(accumulated_extra_samples, bFinalTick);
}
