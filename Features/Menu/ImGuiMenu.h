#pragma once

#include <d3d9.h>
#include <Windows.h>
#include <atomic>

class CImGuiMenu
{
private:
	bool m_bInitialized = false;
	std::atomic<bool> m_bOpen = false;
	int m_nTabIndex = 0;
	
	IDirect3DDevice9* m_pDevice = nullptr;
	HWND m_hWindow = nullptr;
	WNDPROC m_pOriginalWndProc = nullptr;

	bool m_bShowKeybindWindow = true;
	float m_fWindowColor[4] = { 0.0f, 220.0f / 255.0f, 0.0f, 1.0f };

	// Key binding state
	bool m_bAimbotKeyListening = false;

	void DrawAimbotTab();
	void DrawVisualsTab();
	void DrawMiscTab();
	void DrawPlayersTab();
	void DrawConfigTab();
	void DrawAboutTab();
	void DrawKeybindWindow();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	bool Initialize(IDirect3DDevice9* pDevice);
	void Shutdown();
	void Render();
	void Toggle();
	
	void OnDeviceLost();
	void OnDeviceReset();

	bool IsOpen() const { return m_bOpen; }
	bool IsInitialized() const { return m_bInitialized; }
};

namespace F { inline CImGuiMenu ImGuiMenu; }
