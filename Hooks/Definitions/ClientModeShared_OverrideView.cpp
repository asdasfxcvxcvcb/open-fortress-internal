#include "../Hooks.h"
#include "../../Features/Thirdperson/Thirdperson.h"

DEFINE_HOOK(ClientModeShared_OverrideView, void, __fastcall, void* ecx, void* edx, CViewSetup* pSetup)
{
	Func.Original<FN>()(ecx, edx, pSetup);

	// Run thirdperson AFTER original
	F::Thirdperson.Run(pSetup);
}
