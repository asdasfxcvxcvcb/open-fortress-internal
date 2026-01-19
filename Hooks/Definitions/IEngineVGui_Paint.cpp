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

	// Handle F11 key for unload
	static bool bF11Pressed = false;
	if (GetAsyncKeyState(VK_F11) & 0x8000)
	{
		if (!bF11Pressed)
		{
			bF11Pressed = true;
			
			// Unload hooks and cleanup
			G::Entry.Unload();
			
			// Get the module handle BEFORE creating the thread
			HMODULE hModule = GetModuleHandleW(L"necromancer.dll");
			if (!hModule)
				hModule = GetModuleHandleW(L"of.dll");
			
			// Create a thread to free the library after a short delay
			CreateThread(nullptr, 0, [](LPVOID lpParam) -> DWORD {
				Sleep(100); // Give time for cleanup
				FreeLibraryAndExitThread((HMODULE)lpParam, 0);
				return 0;
			}, hModule, 0, nullptr);
		}
	}
	else
	{
		bF11Pressed = false;
	}

	// INSERT key is now handled by ImGui WndProc

	if (mode & PAINT_UIPANELS)
	{
		I::MatSystemSurface->StartDrawing();
		{
			H::Draw.UpdateMatrix(); // Keep this to update screen dimensions
		}
		I::MatSystemSurface->FinishDrawing();
	}
}