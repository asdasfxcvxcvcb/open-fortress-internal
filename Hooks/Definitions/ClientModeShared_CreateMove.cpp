#include "../Hooks.h"

#include "../../Features/EnginePrediction/EnginePrediction.h"
#include "../../Features/Aimbot/Aimbot.h"
#include "../../Features/Thirdperson/Thirdperson.h"
#include "../../Features/Menu/ImGuiMenu.h"

DEFINE_HOOK(ClientModeShared_CreateMove, bool, __fastcall, void* ecx, void* edx, float input_sample_frametime, CUserCmd* cmd)
{
	// Reset per-frame state
	G::bSilentAngles = false;
	G::bPSilentAngles = false;

	if (!cmd || !cmd->command_number)
		return Func.Original<FN>()(ecx, edx, input_sample_frametime, cmd);

	auto pEntity = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pEntity)
		return Func.Original<FN>()(ecx, edx, input_sample_frametime, cmd);

	C_TFPlayer* pLocal = reinterpret_cast<C_TFPlayer*>(pEntity);

	if (Func.Original<FN>()(ecx, edx, input_sample_frametime, cmd)) {
		pLocal->SetLocalViewAngles(cmd->viewangles);
		I::EngineClient->SetViewAngles(cmd->viewangles);
	}

	/*I::Prediction->Update(
		I::ClientState->m_nDeltaTick,
		I::ClientState->m_nDeltaTick > 0,
		I::ClientState->last_command_ack(),
		I::ClientState->lastoutgoingcommand() + chocked
	);*/

	// Cache original angles BEFORE any modifications
	const Vector vOldAngles = cmd->viewangles;
	const float flOldForward = cmd->forwardmove;
	const float flOldSide = cmd->sidemove;

	F::EnginePrediction.Start(pLocal, cmd);
	{
		F::Aimbot.Run(pLocal, cmd);
	}
	F::EnginePrediction.Finish(pLocal);

	{
		Vector forward, right, up;
		U::Math.AngleVectors(vOldAngles, &forward, &right, &up);

		forward.z = right.z = up.x = up.y = 0.f;

		forward.Normalize();
		right.Normalize();
		up.Normalize();

		Vector OldForward, OldRight, OldUp;
		U::Math.AngleVectors(cmd->viewangles, &OldForward, &OldRight, &OldUp);

		OldForward.z = OldRight.z = OldUp.x = OldUp.y = 0.f;

		OldForward.Normalize();
		OldRight.Normalize();
		OldUp.Normalize();

		const float pForward = forward.x * cmd->forwardmove;
		const float yForward = forward.y * cmd->forwardmove;
		const float pSide = right.x * cmd->sidemove;
		const float ySide = right.y * cmd->sidemove;
		const float rUp = up.z * cmd->upmove;

		const float x = OldForward.x * pSide + OldForward.y * ySide + OldForward.x * pForward + OldForward.y * yForward + OldForward.z * rUp;
		const float y = OldRight.x * pSide + OldRight.y * ySide + OldRight.x * pForward + OldRight.y * yForward + OldRight.z * rUp;
		const float z = OldUp.x * ySide + OldUp.y * pSide + OldUp.x * yForward + OldUp.y * pForward + OldUp.z * rUp;

		cmd->forwardmove = std::clamp(x, -450.f, 450.f);
		cmd->sidemove = std::clamp(y, -450.f, 450.f);
		cmd->upmove = std::clamp(z, -450.f, 450.f);
	}

	return false;
}