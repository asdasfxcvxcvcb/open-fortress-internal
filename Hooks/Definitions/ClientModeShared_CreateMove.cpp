#include "../Hooks.h"

#include "../../Features/EnginePrediction/EnginePrediction.h"
#include "../../Features/Aimbot/Aimbot.h"
#include "../../Features/Thirdperson/Thirdperson.h"
#include "../../Features/Menu/ImGuiMenu.h"

DEFINE_HOOK(ClientModeShared_CreateMove, bool, __fastcall, void* ecx, void* edx, float input_sample_frametime, CUserCmd* cmd)
{
	if (!cmd || !cmd->command_number)
		return Func.Original<FN>()(ecx, edx, input_sample_frametime, cmd);

	// Reset per-frame state
	G::bSilentAngles = false;
	G::bPSilentAngles = false;

	// Block mouse input when menu is open, but allow WASD movement
	if (F::ImGuiMenu.IsOpen())
	{
		// Only block mouse movement and buttons, allow WASD
		cmd->buttons = 0;
		cmd->mousedx = 0;
		cmd->mousedy = 0;
		
		// Don't modify forwardmove, sidemove, or upmove - allow movement
		// Return false to prevent view angle changes
		return false;
	}

	// Cache original angles BEFORE any modifications
	const Vector vOldAngles = cmd->viewangles;
	const float flOldForward = cmd->forwardmove;
	const float flOldSide = cmd->sidemove;

	auto pEntity = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pEntity)
		return false;

	C_TFPlayer* pLocal = reinterpret_cast<C_TFPlayer*>(pEntity);

	F::EnginePrediction.Start(cmd);
	{
		F::Aimbot.Run(pLocal, cmd);
	}
	F::EnginePrediction.Finish();

	// PSilent handling - restore view angles but keep cmd angles for server
	if (G::bPSilentAngles)
	{
		// Fix movement to match the new angles
		Vector vMove(cmd->forwardmove, cmd->sidemove, cmd->upmove);
		Vector vAngleDiff = cmd->viewangles - vOldAngles;
		
		// Rotate movement vector
		float flYaw = DEG2RAD(vAngleDiff.y);
		float flCos = cosf(flYaw);
		float flSin = sinf(flYaw);
		
		cmd->forwardmove = (vMove.x * flCos) - (vMove.y * flSin);
		cmd->sidemove = (vMove.x * flSin) + (vMove.y * flCos);
		
		// Restore view angles so player doesn't see the snap
		Vector vRestoreAngles = vOldAngles;
		I::EngineClient->SetViewAngles(vRestoreAngles);
		
		return false; // Don't call original to prevent view update
	}

	return false;
}