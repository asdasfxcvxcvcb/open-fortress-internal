#include "../Hooks.h"

#include "../../Entry/Entry.h"
#include "../../Features/ESP/ESP.h"
#include "../../Features/Menu/ImGuiMenu.h"
#include "../../Features/Aimbot/Aimbot.h"
#include "../../Util/DirectX/DirectX.h"

DEFINE_HOOK(IEngineVGui_Paint, void, __fastcall, void* ecx, void* edx, int mode)
{
	Func.Original<FN>()(ecx, edx, mode);

	// Lazy initialize DirectX hooks on first paint
	static bool bDirectXInitialized = false;
	if (!bDirectXInitialized)
	{
		U::DirectX.Initialize();
		bDirectXInitialized = true;
	}

	// INSERT key is now handled by ImGui WndProc

	if (mode & PAINT_UIPANELS)
	{
		H::Draw.UpdateMatrix(); // Keep this to update screen dimensions
	}
}