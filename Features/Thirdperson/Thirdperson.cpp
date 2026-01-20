#include "Thirdperson.h"
#include "../Vars.h"

void CThirdperson::Run(CViewSetup* pSetup)
{
	if (!pSetup)
		return;

	auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal)
		return;

	C_TFPlayer* pPlayer = reinterpret_cast<C_TFPlayer*>(pLocal);
	
	static bool bLastState = false;

	// Force first person when dead
	if (!pPlayer->IsAlive())
	{
		if (bLastState)
		{
			I::EngineClient->ClientCmd_Unrestricted("firstperson");
			bLastState = false;
		}
		return;
	}

	// Set camera mode based on setting
	if (Vars::Misc::Thirdperson)
	{
		// Enable thirdperson if state changed
		if (!bLastState)
		{
			I::EngineClient->ClientCmd_Unrestricted("thirdperson");
			bLastState = true;
		}
		
		// Apply camera offset
		Vector vForward = {}, vRight = {}, vUp = {};
		U::Math.AngleVectors(pSetup->angles, &vForward, &vRight, &vUp);

		const Vector vOffset = (vForward * Vars::Misc::ThirdpersonBack)
			- (vRight * Vars::Misc::ThirdpersonRight)
			- (vUp * Vars::Misc::ThirdpersonUp);

		const Vector vDesiredOrigin = pSetup->origin - vOffset;

		trace_t trace;
		CTraceFilterWorldOnly filter;
		U::Trace.TraceRay(pSetup->origin, vDesiredOrigin, MASK_SOLID, &filter, &trace);

		pSetup->origin -= vOffset * trace.fraction;
	}
	else
	{
		// Disable thirdperson if state changed
		if (bLastState)
		{
			I::EngineClient->ClientCmd_Unrestricted("firstperson");
			bLastState = false;
		}
	}
}
