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

	bool m_bShowKeybindWindow = true;
	float m_fWindowColor[4] = { 0.0f, 220.0f / 255.0f, 0.0f, 1.0f };
	
	// Color customization
	float m_fMenuTextColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float m_fMenuBackgroundColor[4] = { 0.05f, 0.05f, 0.05f, 0.95f };
	float m_fAimbotFOVColor[4] = { 1.0f, 1.0f, 1.0f, 0.4f };
	float m_fMenuAccentColor[4] = { 0.0f, 220.0f / 255.0f, 0.0f, 1.0f };

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
	
	// Color getters for config
	const float* GetMenuBackgroundColor() const { return m_fMenuBackgroundColor; }
	const float* GetMenuTextColor() const { return m_fMenuTextColor; }
	const float* GetMenuAccentColor() const { return m_fMenuAccentColor; }
	const float* GetWindowColor() const { return m_fWindowColor; }
	const float* GetAimbotFOVColor() const { return m_fAimbotFOVColor; }
	
	// Color setters for config
	void SetMenuBackgroundColor(float r, float g, float b, float a) { m_fMenuBackgroundColor[0] = r; m_fMenuBackgroundColor[1] = g; m_fMenuBackgroundColor[2] = b; m_fMenuBackgroundColor[3] = a; }
	void SetMenuTextColor(float r, float g, float b, float a) { m_fMenuTextColor[0] = r; m_fMenuTextColor[1] = g; m_fMenuTextColor[2] = b; m_fMenuTextColor[3] = a; }
	void SetMenuAccentColor(float r, float g, float b, float a) { m_fMenuAccentColor[0] = r; m_fMenuAccentColor[1] = g; m_fMenuAccentColor[2] = b; m_fMenuAccentColor[3] = a; }
	void SetWindowColor(float r, float g, float b, float a) { m_fWindowColor[0] = r; m_fWindowColor[1] = g; m_fWindowColor[2] = b; m_fWindowColor[3] = a; }
	void SetAimbotFOVColor(float r, float g, float b, float a) { m_fAimbotFOVColor[0] = r; m_fAimbotFOVColor[1] = g; m_fAimbotFOVColor[2] = b; m_fAimbotFOVColor[3] = a; }
};

namespace F { inline CMenu Menu; }
