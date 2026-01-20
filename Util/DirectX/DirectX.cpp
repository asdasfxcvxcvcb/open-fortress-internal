#include "DirectX.h"
#include "../Hook/Hook.h"
#include "../../SDK/SDK.h"

typedef HRESULT(__stdcall* Present_t)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
typedef HRESULT(__stdcall* Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

static Present_t oPresent = nullptr;
static Reset_t oReset = nullptr;

extern void ImGuiPresentHook(IDirect3DDevice9* pDevice);
extern void ImGuiResetHook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pParams, bool bBefore);

HRESULT __stdcall hkPresent(IDirect3DDevice9* pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
	ImGuiPresentHook(pDevice);
	return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
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
	if (!I::MatSystemSurface)
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] MatSystemSurface not initialized!\n");
		return false;
	}

	// Get the D3D9 device directly from the surface interface vtable
	// The device is stored at offset in the surface object
	void** pSurfaceVTable = *reinterpret_cast<void***>(I::MatSystemSurface);
	
	// Create a temporary dummy device to get vtable indices
	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D)
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Failed to create D3D9 interface!\n");
		return false;
	}

	D3DPRESENT_PARAMETERS d3dpp = {};
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.hDeviceWindow = GetDesktopWindow();

	IDirect3DDevice9* pDummyDevice = nullptr;
	HRESULT hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

	if (FAILED(hr) || !pDummyDevice)
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Failed to create dummy device!\n");
		pD3D->Release();
		return false;
	}

	// Get vtable from dummy device
	m_pDeviceVTable = *reinterpret_cast<void***>(pDummyDevice);
	
	// Cleanup dummy device
	pDummyDevice->Release();
	pD3D->Release();

	if (!m_pDeviceVTable)
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Failed to get device vtable!\n");
		return false;
	}

	// Hook Present (index 17) instead of EndScene
	oPresent = (Present_t)m_pDeviceVTable[17];
	MH_STATUS status = MH_CreateHook(m_pDeviceVTable[17], &hkPresent, reinterpret_cast<void**>(&oPresent));
	
	if (status != MH_OK)
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Present hook failed: %d\n", status);
		return false;
	}
	
	MH_EnableHook(m_pDeviceVTable[17]);

	// Hook Reset (index 16)
	oReset = (Reset_t)m_pDeviceVTable[16];
	status = MH_CreateHook(m_pDeviceVTable[16], &hkReset, reinterpret_cast<void**>(&oReset));
	
	if (status != MH_OK)
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[DirectX] Reset hook failed: %d\n", status);
		return false;
	}
	
	MH_EnableHook(m_pDeviceVTable[16]);

	if (I::Cvar)
		I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[DirectX] Hooks initialized (Present + Reset)\n");
	
	return true;
}
