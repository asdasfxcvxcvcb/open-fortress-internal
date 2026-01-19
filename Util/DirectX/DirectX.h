#pragma once

#include <d3d9.h>
#include <Windows.h>

class CUtil_DirectX
{
public:
	bool Initialize();
	IDirect3DDevice9* GetDevice() { return m_pDevice; }

private:
	IDirect3DDevice9* m_pDevice = nullptr;
	void** m_pDeviceVTable = nullptr;
};

namespace U { inline CUtil_DirectX DirectX; }
