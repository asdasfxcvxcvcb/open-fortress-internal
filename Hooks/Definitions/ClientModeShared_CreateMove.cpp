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
	G::bPSilentAngles = false;
	G::Attacking = 0;
	G::bCanPrimaryAttack = false;
	G::bCanSecondaryAttack = false;
	G::bReloading = false;

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
	C_BaseCombatWeapon* pWeapon = pLocal->GetActiveWeapon();

	F::EnginePrediction.Start(cmd);
	{
		if (pWeapon)
		{
			float flServerTime = pLocal->m_nTickBase() * I::GlobalVarsBase->interval_per_tick;
			float flTimeDiff = pWeapon->m_flNextPrimaryAttack() - flServerTime;
			G::bCanPrimaryAttack = (flTimeDiff <= 0.5f);
			G::bCanSecondaryAttack = (pWeapon->m_flNextSecondaryAttack() <= flServerTime);
			
			if (pWeapon->m_iClip1() <= 0 && pWeapon->GetMaxClip1() > 0)
				G::bReloading = true;
		}
		
		F::AutoStrafe.Run(pLocal, cmd);
		F::Aimbot.Run(pLocal, cmd);
		F::Chat.Run();
	}
	F::EnginePrediction.Finish();

	if (pSendPacket)
	{
		static bool bWasSet = false;
		
		if (G::bPSilentAngles)
		{
			Vector vMove(flOldForward, flOldSide, cmd->upmove);
			Vector vAngleDiff = cmd->viewangles - vOldAngles;
			
			float flYaw = DEG2RAD(vAngleDiff.y);
			float flCos = cosf(flYaw);
			float flSin = sinf(flYaw);
			
			cmd->forwardmove = (vMove.x * flCos) - (vMove.y * flSin);
			cmd->sidemove = (vMove.x * flSin) + (vMove.y * flCos);
			
			*pSendPacket = false;
			bWasSet = true;
		}
		else if (bWasSet)
		{
			*pSendPacket = true;
			cmd->viewangles = vOldAngles;
			cmd->forwardmove = flOldForward;
			cmd->sidemove = flOldSide;
			bWasSet = false;
		}
	}

	if (G::bSilentAngles || G::bPSilentAngles)
	{
		Vector vRestoreAngles = vOldAngles;
		I::EngineClient->SetViewAngles(vRestoreAngles);
	}

	return false;
}