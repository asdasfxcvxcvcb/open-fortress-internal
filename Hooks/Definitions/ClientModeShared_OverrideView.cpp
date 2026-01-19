#include "../Hooks.h"
#include "../../Features/Vars.h"

DEFINE_HOOK(ClientModeShared_OverrideView, void, __fastcall, void* ecx, void* edx, CViewSetup* pSetup)
{
	// Call original
	Func.Original<FN>()(ecx, edx, pSetup);

	// Third person camera
	if (pSetup)
	{
		// Get local player
		if (!I::ClientEntityList || !I::EngineClient)
			return;

		auto pEntity = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
		if (!pEntity)
			return;

		C_TFPlayer* pLocal = pEntity->As<C_TFPlayer*>();
		if (!pLocal || !pLocal->IsAlive())
			return;

		if (Vars::Misc::Thirdperson)
		{
			// Call ThirdPersonSwitch via signature to enable third person
			pLocal->ThirdPersonSwitchSig();
			
			// Calculate third person camera position
			Vector vForward, vRight, vUp;
			U::Math.AngleVectors(pSetup->angles, &vForward, &vRight, &vUp);

			// Apply offsets (forward is negative to go behind player)
			Vector vOffset = (vForward * Vars::Misc::ThirdpersonBack)
				- (vRight * Vars::Misc::ThirdpersonRight)
				- (vUp * Vars::Misc::ThirdpersonUp);

			Vector vDesiredOrigin = pSetup->origin - vOffset;
			
			// Trace to make sure we don't go through walls
			trace_t trace;
			CTraceFilterHitscan filter;
			filter.pSkip = pLocal;
			U::Trace.TraceRay(pSetup->origin, vDesiredOrigin, MASK_SOLID, &filter, &trace);
			
			pSetup->origin -= vOffset * trace.fraction;
		}
	}
}
