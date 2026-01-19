#include "DirectX.h"
#include "../Hook/Hook.h"
#include "../../SDK/SDK.h"
#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")

typedef HRESULT(__stdcall* EndScene_t)(IDirect3DDevice9*);
typedef HRESULT(__stdcall* Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

static EndScene_t oEndScene = nullptr;
static Reset_t oReset = nullptr;

extern void ImGuiEndSceneHook(IDirect3DDevice9* pDevice);
extern void ImGuiResetHook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pParams, bool bBefore);

HRESULT __stdcall hkEndScene(IDirect3DDevice9* pDevice)
{
	ImGuiEndSceneHook(pDevice);
	return oEndScene(pDevice);
}

HRESULT __stdcall hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pParams)
{
	ImGuiResetHook(pDevice, pParams, true);
	HRESULT result = oReset(pDevice, pParams);
	ImGuiResetHook(pDevice, pParams, false);
	return result;
}

bool CUtil_DirectX::Initialize()
{
	printf("[DirectX] Initializing DirectX hooks...\n");
	
	if (I::Cvar)
		I::Cvar->ConsoleColorPrintf({ 255, 255, 0, 255 }, "[DirectX] Starting initialization...\n");

	try
	{
		// Try to find the game's D3D9 device instead of creating a dummy one
		// Pattern scan for the device pointer in shaderapidx9.dll
		HMODULE hShaderAPI = GetModuleHandleA("shaderapidx9.dll");
		if (!hShaderAPI)
		{
			if (I::Cvar)
				I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Failed to find shaderapidx9.dll\n");
			return false;
		}

		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[DirectX] Found shaderapidx9.dll\n");

		// Pattern scan for D3D9 device pointer
		// This pattern looks for the device pointer in the shader API
		DWORD* pDevice = nullptr;
		
		// Try multiple patterns to find the device
		const char* pattern1 = "\xA1\x00\x00\x00\x00\x50\x8B\x08\xFF\x51\x0C"; // mov eax, [device]; push eax; mov ecx, [eax]; call [ecx+0Ch]
		const char* mask1 = "x????xxxxxx";
		
		MODULEINFO modInfo;
		if (GetModuleInformation(GetCurrentProcess(), hShaderAPI, &modInfo, sizeof(MODULEINFO)))
		{
			DWORD baseAddr = (DWORD)modInfo.lpBaseOfDll;
			DWORD size = modInfo.SizeOfImage;
			
			// Simple pattern scan
			for (DWORD i = 0; i < size - 11; i++)
			{
				bool found = true;
				for (int j = 0; j < 11; j++)
				{
					if (mask1[j] == 'x' && ((char*)baseAddr)[i + j] != pattern1[j])
					{
						found = false;
						break;
					}
				}
				
				if (found)
				{
					// Found pattern, extract device pointer
					pDevice = *(DWORD**)(baseAddr + i + 1);
					break;
				}
			}
		}

		// If pattern scan failed, try alternative method: hook Present and get device from there
		if (!pDevice)
		{
			if (I::Cvar)
				I::Cvar->ConsoleColorPrintf({ 255, 255, 0, 255 }, "[DirectX] Pattern scan failed, using fallback method...\n");
			
			// Create a minimal dummy device with different parameters
			IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
			if (!pD3D)
			{
				if (I::Cvar)
					I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Failed to create D3D9 interface!\n");
				return false;
			}

			// Try with NULL window and different creation flags
			D3DPRESENT_PARAMETERS d3dpp = {};
			d3dpp.Windowed = TRUE;
			d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
			d3dpp.hDeviceWindow = FindWindowA("Valve001", nullptr);
			
			if (!d3dpp.hDeviceWindow)
				d3dpp.hDeviceWindow = GetDesktopWindow();

			IDirect3DDevice9* pDummyDevice = nullptr;
			HRESULT hr = pD3D->CreateDevice(
				D3DADAPTER_DEFAULT,
				D3DDEVTYPE_NULLREF, // Use NULL reference device instead of HAL
				d3dpp.hDeviceWindow,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
				&d3dpp,
				&pDummyDevice
			);

			if (FAILED(hr) || !pDummyDevice)
			{
				if (I::Cvar)
					I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Failed to create dummy device! HRESULT: 0x%X\n", hr);
				pD3D->Release();
				return false;
			}

			if (I::Cvar)
				I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[DirectX] Dummy device created\n");

			// Get VTable
			m_pDeviceVTable = *reinterpret_cast<void***>(pDummyDevice);
			
			// Cleanup
			pDummyDevice->Release();
			pD3D->Release();
		}
		else
		{
			// Got device from pattern scan
			IDirect3DDevice9* pGameDevice = (IDirect3DDevice9*)*pDevice;
			if (!pGameDevice)
			{
				if (I::Cvar)
					I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Game device is null!\n");
				return false;
			}
			
			m_pDeviceVTable = *reinterpret_cast<void***>(pGameDevice);
			
			if (I::Cvar)
				I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[DirectX] Found game device via pattern scan\n");
		}

		if (!m_pDeviceVTable)
		{
			if (I::Cvar)
				I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] VTable is null!\n");
			return false;
		}

		printf("[DirectX] VTable: %p\n", m_pDeviceVTable);
		printf("[DirectX] EndScene: %p\n", m_pDeviceVTable[42]);
		printf("[DirectX] Reset: %p\n", m_pDeviceVTable[16]);

		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[DirectX] VTable obtained: %p\n", m_pDeviceVTable);

		// Hook EndScene (index 42)
		oEndScene = (EndScene_t)m_pDeviceVTable[42];
		MH_STATUS status = MH_CreateHook(m_pDeviceVTable[42], &hkEndScene, reinterpret_cast<void**>(&oEndScene));
		printf("[DirectX] EndScene hook status: %d\n", status);
		
		if (status != MH_OK)
		{
			if (I::Cvar)
				I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] EndScene hook failed with status: %d\n", status);
			return false;
		}
		
		MH_EnableHook(m_pDeviceVTable[42]);

		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[DirectX] EndScene hooked\n");

		// Hook Reset (index 16)
		oReset = (Reset_t)m_pDeviceVTable[16];
		status = MH_CreateHook(m_pDeviceVTable[16], &hkReset, reinterpret_cast<void**>(&oReset));
		printf("[DirectX] Reset hook status: %d\n", status);
		
		if (status != MH_OK)
		{
			if (I::Cvar)
				I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Reset hook failed with status: %d\n", status);
			return false;
		}
		
		MH_EnableHook(m_pDeviceVTable[16]);

		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[DirectX] Reset hooked\n");

		printf("[DirectX] DirectX hooks initialized successfully!\n");
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[DirectX] Initialization complete!\n");
		
		return true;
	}
	catch (...)
	{
		printf("[DirectX] Exception during initialization!\n");
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Exception during initialization!\n");
		return false;
	}
}
