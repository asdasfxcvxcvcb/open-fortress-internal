#include "Menu.h"
#include <d3d9.h>
#include <stdio.h>

void ImGuiPresentHook(IDirect3DDevice9* pDevice)
{
	static bool bInit = false;
	if (!bInit)
	{
		printf("[ImGuiBridge] Present called, initializing ImGui with device: %p\n", pDevice);
		bool result = F::Menu.Initialize(pDevice);
		printf("[ImGuiBridge] ImGui initialization result: %d\n", result);
		bInit = true;
	}

	// Only render if initialized
	if (F::Menu.IsInitialized())
	{
		F::Menu.Render();
	}
}

void ImGuiResetHook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pParams, bool bBefore)
{
	printf("[ImGuiBridge] Reset called, bBefore: %d\n", bBefore);
	
	// Only call hooks if initialized
	if (F::Menu.IsInitialized())
	{
		if (bBefore)
			F::Menu.OnDeviceLost();
		else
			F::Menu.OnDeviceReset();
	}
}
