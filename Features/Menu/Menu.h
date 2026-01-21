#pragma once

#include <d3d9.h>
#include <Windows.h>
#include <atomic>
#include <unordered_map>
#include <string>

class CMenu
{
private:
	bool m_bInitialized = false;
	std::atomic<bool> m_bOpen = false;
	
	IDirect3DDevice9* m_pDevice = nullptr;
	HWND m_hWindow = nullptr;
	WNDPROC m_pOriginalWndProc = nullptr;

	// Bind system state
	char m_szBindName[11] = "";
	bool m_bBindKeyListening = false;
	int m_nTempBindKey = 0;
	int m_nBindType = 0; // 0 = hold, 1 = toggle
	int m_nEditingBindIndex = -1;
	bool m_bInEditMode = false;
	int m_nEditingBindPropsIndex = -1; // For editing bind key/type
	
	// Store original settings before entering edit mode
	std::unordered_map<std::string, bool> m_mOriginalBoolSettings;
	std::unordered_map<std::string, int> m_mOriginalIntSettings;
	std::unordered_map<std::string, float> m_mOriginalFloatSettings;

	void DrawAimbotTab();
	void DrawVisualsTab();
	void DrawMiscTab();
	void DrawPlayersTab();
	void DrawConfigTab();
	void DrawBindsTab();
	void DrawKeybindWindow();
	
	void CaptureBindSettings(int bindIndex);
	void ApplyBindSettings(int bindIndex);
	void CheckBindActivation();

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

namespace F { inline CMenu Menu; }
