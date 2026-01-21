#include "../Hooks.h"

#include "../../Features/EnginePrediction/EnginePrediction.h"
#include "../../Features/Aimbot/Aimbot.h"
#include "../../Features/Thirdperson/Thirdperson.h"
#include "../../Features/Menu/Menu.h"
#include "../../Features/Chat/Chat.h"
#include "../../Features/AutoStrafe/AutoStrafe.h"
#include "../../vendor/imgui/imgui.h"

DEFINE_HOOK(ClientModeShared_CreateMove, bool, __fastcall, void* ecx, void* edx, float input_sample_frametime, CUserCmd* cmd)
{
	if (!cmd || !cmd->command_number)
		return Func.Original<FN>()(ecx, edx, input_sample_frametime, cmd);

	// Get SendPacket pointer - it's on the stack
	bool* pSendPacket = nullptr;
#ifdef _WIN64
	// 64-bit: Use intrinsic to get frame pointer
	pSendPacket = reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(_AddressOfReturnAddress()) + 0x8);
#else
	// 32-bit: Use inline assembly
	__asm
	{
		mov eax, ebp
		sub eax, 0x1C
		mov pSendPacket, eax
	}
#endif

	// Reset per-frame state
	G::bSilentAngles = false;
	G::bPSilentAngles = false;

	// Block mouse input when menu is open, but allow WASD movement
	// Also block ALL input when typing in text fields (ImGui WantCaptureKeyboard)
	ImGuiIO& io = ImGui::GetIO();
	if (F::Menu.IsOpen())
	{
		// If typing in a text field, block ALL input including movement
		if (io.WantCaptureKeyboard)
		{
			cmd->buttons = 0;
			cmd->forwardmove = 0;
			cmd->sidemove = 0;
			cmd->upmove = 0;
			cmd->mousedx = 0;
			cmd->mousedy = 0;
			return false;
		}
		
		// Otherwise just block mouse/buttons but allow WASD
		cmd->buttons = 0;
		cmd->mousedx = 0;
		cmd->mousedy = 0;
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
		F::Chat.Run();
		F::AutoStrafe.Run(pLocal, cmd);
	}
	F::EnginePrediction.Finish();

	// PSilent handling - choke packet, then send with restored angles next frame
	if (pSendPacket)
	{
		static bool bWasSet = false;
		
		if (G::bPSilentAngles)
		{
			// Fix movement to match the new angles
			Vector vMove(flOldForward, flOldSide, cmd->upmove);
			Vector vAngleDiff = cmd->viewangles - vOldAngles;
			
			// Rotate movement vector
			float flYaw = DEG2RAD(vAngleDiff.y);
			float flCos = cosf(flYaw);
			float flSin = sinf(flYaw);
			
			cmd->forwardmove = (vMove.x * flCos) - (vMove.y * flSin);
			cmd->sidemove = (vMove.x * flSin) + (vMove.y * flCos);
			
			// Choke this packet - server gets the aim angles
			*pSendPacket = false;
			bWasSet = true;
		}
		else if (bWasSet)
		{
			// Next frame after psilent - send packet with restored angles
			*pSendPacket = true;
			cmd->viewangles = vOldAngles;
			cmd->forwardmove = flOldForward;
			cmd->sidemove = flOldSide;
			bWasSet = false;
		}
	}

	// Restore view angles for all silent aim modes
	if (G::bSilentAngles || G::bPSilentAngles)
	{
		Vector vRestoreAngles = vOldAngles;
		I::EngineClient->SetViewAngles(vRestoreAngles);
	}

	return false;
}