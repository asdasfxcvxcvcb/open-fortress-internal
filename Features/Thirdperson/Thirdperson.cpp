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
	if (!pPlayer || !pPlayer->IsAlive())
		return;

	// Toggle third person
	if (Vars::Misc::Thirdperson)
	{
		I::Input->CAM_ToThirdPerson();
	}
	else
	{
		I::Input->CAM_ToFirstPerson();
		return;
	}

	// Calculate camera offset
	Vector vForward, vRight, vUp;
	U::Math.AngleVectors(pSetup->angles, &vForward, &vRight, &vUp);

	Vector vOffset = (vForward * Vars::Misc::ThirdpersonBack)
		- (vRight * Vars::Misc::ThirdpersonRight)
		- (vUp * Vars::Misc::ThirdpersonUp);

	Vector vDesiredOrigin = pSetup->origin - vOffset;

	// Trace to prevent camera going through walls
	trace_t trace;
	CTraceFilterWorldOnly filter;
	U::Trace.TraceRay(pSetup->origin, vDesiredOrigin, MASK_SOLID, &filter, &trace);

	pSetup->origin -= vOffset * trace.fraction;
}
