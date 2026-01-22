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

	G::bSilentAngles = false;

	ImGuiIO& io = ImGui::GetIO();
	if (F::Menu.IsOpen())
	{
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
		
		cmd->buttons = 0;
		cmd->mousedx = 0;
		cmd->mousedy = 0;
		return false;
	}

	const Vector vOldAngles = cmd->viewangles;
	const float flOldForward = cmd->forwardmove;
	const float flOldSide = cmd->sidemove;

	auto pEntity = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pEntity)
		return false;

	C_TFPlayer* pLocal = reinterpret_cast<C_TFPlayer*>(pEntity);

	F::EnginePrediction.Start(cmd);
	{
		F::AutoStrafe.Run(pLocal, cmd);
		F::Aimbot.Run(pLocal, cmd);
		F::Chat.Run();
	}
	F::EnginePrediction.Finish();

	if (pSendPacket)
	{
		// PSilent is removed - not needed in TF2
	}

	if (G::bSilentAngles)
	{
		Vector vRestoreAngles = vOldAngles;
		I::EngineClient->SetViewAngles(vRestoreAngles);
	}

	return false;
}