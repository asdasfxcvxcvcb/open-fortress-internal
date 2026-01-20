#include "ImGuiMenu.h"
#include "../Vars.h"
#include "../Aimbot/Aimbot.h"
#include "../ESP/ESP.h"
#include "../Config/Config.h"
#include "../../vendor/imgui/imgui.h"
#include "../../vendor/imgui/imgui_impl_win32.h"
#include "../../vendor/imgui/imgui_impl_dx9.h"
#include <fstream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static CImGuiMenu* g_pMenuInstance = nullptr;

LRESULT CALLBACK CImGuiMenu::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!g_pMenuInstance)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	// Toggle menu with INSERT key (handle before anything else)
	if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
	{
		g_pMenuInstance->Toggle();
		return 0;
	}

	// Block all mouse messages when menu is open
	if (g_pMenuInstance->m_bOpen.load())
	{
		// Handle mouse button binding BEFORE blocking
		if (g_pMenuInstance->m_bAimbotKeyListening)
		{
			if (uMsg == WM_LBUTTONDOWN)
			{
				Vars::Aimbot::AimbotKey = VK_LBUTTON;
				g_pMenuInstance->m_bAimbotKeyListening = false;
				return 1;
			}
			else if (uMsg == WM_RBUTTONDOWN)
			{
				Vars::Aimbot::AimbotKey = VK_RBUTTON;
				g_pMenuInstance->m_bAimbotKeyListening = false;
				return 1;
			}
			else if (uMsg == WM_MBUTTONDOWN)
			{
				Vars::Aimbot::AimbotKey = VK_MBUTTON;
				g_pMenuInstance->m_bAimbotKeyListening = false;
				return 1;
			}
			else if (uMsg == WM_XBUTTONDOWN)
			{
				// wParam contains which X button (XBUTTON1 or XBUTTON2)
				int xButton = GET_XBUTTON_WPARAM(wParam);
				Vars::Aimbot::AimbotKey = (xButton == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
				g_pMenuInstance->m_bAimbotKeyListening = false;
				return 1;
			}
		}
		
		// Block all mouse input from reaching the game
		if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
		{
			// Let ImGui handle it first
			ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
			// Then block it from the game
			return 1;
		}

		// Handle ImGui input for keyboard
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;
	}

	// Handle key binding for keyboard keys
	if (uMsg == WM_KEYDOWN && g_pMenuInstance->m_bOpen.load())
	{
		if (g_pMenuInstance->m_bAimbotKeyListening)
		{
			Vars::Aimbot::AimbotKey = wParam;
			g_pMenuInstance->m_bAimbotKeyListening = false;
			return 0;
		}
	}

	// Call original window procedure
	return CallWindowProc(g_pMenuInstance->m_pOriginalWndProc, hWnd, uMsg, wParam, lParam);
}

bool CImGuiMenu::Initialize(IDirect3DDevice9* pDevice)
{
	printf("[ImGuiMenu] Initialize called with device: %p\n", pDevice);

	if (m_bInitialized)
	{
		printf("[ImGuiMenu] Already initialized\n");
		return true;
	}

	m_pDevice = pDevice;
	if (!m_pDevice)
	{
		printf("[ImGuiMenu] Device is null!\n");
		return false;
	}

	// Get window handle
	D3DDEVICE_CREATION_PARAMETERS params;
	if (FAILED(m_pDevice->GetCreationParameters(&params)))
	{
		printf("[ImGuiMenu] Failed to get creation parameters!\n");
		return false;
	}

	m_hWindow = params.hFocusWindow;
	if (!m_hWindow)
	{
		printf("[ImGuiMenu] Window handle is null!\n");
		return false;
	}

	printf("[ImGuiMenu] Window handle: %p\n", m_hWindow);

	g_pMenuInstance = this;

	// Hook window procedure
	m_pOriginalWndProc = (WNDPROC)SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
	if (!m_pOriginalWndProc)
	{
		printf("[ImGuiMenu] Failed to hook window procedure!\n");
		return false;
	}

	printf("[ImGuiMenu] Window procedure hooked\n");

	// Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// Don't set NoMouseCursorChange - we want to control the cursor
	io.MouseDrawCursor = false; // Will be enabled when menu opens
	io.WantCaptureMouse = false;
	io.WantCaptureKeyboard = false;

	printf("[ImGuiMenu] ImGui context created\n");

	// Setup style
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 8.0f;
	style.FrameRounding = 4.0f;
	style.FrameBorderSize = 0.0f;
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.95f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(m_fWindowColor[0], m_fWindowColor[1], m_fWindowColor[2], m_fWindowColor[3]);

	// Setup backends
	if (!ImGui_ImplWin32_Init(m_hWindow))
	{
		printf("[ImGuiMenu] ImGui_ImplWin32_Init failed!\n");
		Shutdown();
		return false;
	}

	printf("[ImGuiMenu] Win32 backend initialized\n");

	if (!ImGui_ImplDX9_Init(m_pDevice))
	{
		printf("[ImGuiMenu] ImGui_ImplDX9_Init failed!\n");
		Shutdown();
		return false;
	}

	printf("[ImGuiMenu] DX9 backend initialized\n");

	m_bInitialized = true;
	
	if (I::Cvar)
		I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[ImGuiMenu] Initialized successfully!\n");
	
	return true;
}

void CImGuiMenu::Shutdown()
{
	if (!m_bInitialized)
		return;

	printf("[ImGuiMenu] Shutting down...\n");

	// Mark as not initialized FIRST to stop rendering
	m_bInitialized = false;
	m_bOpen.store(false);

	// Small delay to ensure no more render calls
	Sleep(50);

	// Restore window procedure
	if (m_hWindow && m_pOriginalWndProc)
	{
		SetWindowLongPtr(m_hWindow, GWLP_WNDPROC, (LONG_PTR)m_pOriginalWndProc);
		m_pOriginalWndProc = nullptr;
	}

	// Cleanup ImGui
	try
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
	catch (...)
	{
		printf("[ImGuiMenu] Exception during ImGui cleanup\n");
	}

	m_pDevice = nullptr;
	g_pMenuInstance = nullptr;
	
	printf("[ImGuiMenu] Shutdown complete\n");
}

void CImGuiMenu::OnDeviceLost()
{
	if (m_bInitialized)
		ImGui_ImplDX9_InvalidateDeviceObjects();
}

void CImGuiMenu::OnDeviceReset()
{
	if (m_bInitialized)
		ImGui_ImplDX9_CreateDeviceObjects();
}

void CImGuiMenu::Toggle()
{
	bool expected = m_bOpen.load();
	m_bOpen.store(!expected);
	
	// Update ImGui IO flags
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDrawCursor = m_bOpen.load();
	io.WantCaptureMouse = m_bOpen.load();
	io.WantCaptureKeyboard = m_bOpen.load();
}

static const char* GetKeyNameImGui(int vk)
{
	static char keyName[32];
	switch (vk)
	{
	case 0: return "None";
	case VK_LBUTTON: return "Left Mouse";
	case VK_RBUTTON: return "Right Mouse";
	case VK_MBUTTON: return "Middle Mouse";
	case VK_XBUTTON1: return "Mouse 4";
	case VK_XBUTTON2: return "Mouse 5";
	case VK_SHIFT: return "Shift";
	case VK_CONTROL: return "Ctrl";
	case VK_MENU: return "Alt";
	case VK_SPACE: return "Space";
	case VK_TAB: return "Tab";
	case VK_CAPITAL: return "Caps Lock";
	case VK_F1: return "F1";
	case VK_F2: return "F2";
	case VK_F3: return "F3";
	case VK_F4: return "F4";
	case VK_F5: return "F5";
	case VK_F6: return "F6";
	case VK_F7: return "F7";
	case VK_F8: return "F8";
	case VK_F9: return "F9";
	case VK_F10: return "F10";
	case VK_F11: return "F11";
	case VK_F12: return "F12";
	default:
		if (vk >= 'A' && vk <= 'Z')
		{
			sprintf_s(keyName, "%c", (char)vk);
			return keyName;
		}
		if (vk >= '0' && vk <= '9')
		{
			sprintf_s(keyName, "%c", (char)vk);
			return keyName;
		}
		return "Unknown";
	}
}

void CImGuiMenu::Render()
{
	if (!m_bInitialized)
		return;

	// Start ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Get background draw list for rendering overlays
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	
	if (drawList)
	{
		// Only draw aimbot visuals if we're connected and in a game
		bool bShouldDrawAimbotVisuals = false;
		if (I::EngineClient && I::ClientEntityList && I::EngineClient->IsConnected())
		{
			auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
			if (pLocal && pLocal->As<C_TFPlayer*>()->m_iTeamNum() >= 2)
			{
				bShouldDrawAimbotVisuals = true;
			}
		}
		
		// Draw FOV circle using ImGui DrawList (DirectX rendering) - only for FOV-based targeting
		if (bShouldDrawAimbotVisuals && Vars::Aimbot::Enabled && Vars::Aimbot::DrawFOV && Vars::Aimbot::TargetSelection == 1)
		{
			// Get screen dimensions
			ImGuiIO& io = ImGui::GetIO();
			float centerX = io.DisplaySize.x / 2.0f;
			float centerY = io.DisplaySize.y / 2.0f;

			// Calculate radius based on FOV
			float fovRadians = (Vars::Aimbot::FOV * 3.14159f) / 180.0f;
			float radius = (io.DisplaySize.y / 2.0f) * tanf(fovRadians / 2.0f);

			// Draw the FOV circle with ImGui
			ImU32 circleColor = IM_COL32(255, 255, 255, 100);
			drawList->AddCircle(ImVec2(centerX, centerY), radius, circleColor, 64, 2.0f);
		}

		// Render ESP through ImGui DrawList
		F::ESP.Render();
	}

	// Draw keybind window if enabled (only in-game)
	bool bShouldDrawKeybind = false;
	if (I::EngineClient && I::ClientEntityList && I::EngineClient->IsConnected())
	{
		auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
		if (pLocal && pLocal->As<C_TFPlayer*>()->m_iTeamNum() >= 2)
		{
			bShouldDrawKeybind = true;
		}
	}
	
	if (m_bShowKeybindWindow && bShouldDrawKeybind)
	{
		DrawKeybindWindow();
	}

	// Draw main menu if open
	if (m_bOpen.load())
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TitleBg] = ImVec4(m_fWindowColor[0], m_fWindowColor[1], m_fWindowColor[2], m_fWindowColor[3]);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(m_fWindowColor[0], m_fWindowColor[1], m_fWindowColor[2], m_fWindowColor[3]);

		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
		bool bOpen = m_bOpen.load();
		ImGui::Begin("Necromancer", &bOpen, ImGuiWindowFlags_NoCollapse);
		if (!bOpen) m_bOpen.store(false);

		if (ImGui::BeginTabBar("MenuTabs"))
		{
			if (ImGui::BeginTabItem("Aimbot"))
			{
				DrawAimbotTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Visuals"))
			{
				DrawVisualsTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Players"))
			{
				DrawPlayersTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Misc"))
			{
				DrawMiscTab();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Config"))
			{
				DrawConfigTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	// Render
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void CImGuiMenu::DrawAimbotTab()
{
	ImGui::Checkbox("Enable Aimbot", &Vars::Aimbot::Enabled);
	ImGui::Separator();

	// Aimbot mode
	const char* modes[] = { "Plain", "Smooth", "Silent" };
	ImGui::Combo("Aim Mode", &Vars::Aimbot::Mode, modes, IM_ARRAYSIZE(modes));

	// Smooth amount (only if smooth mode)
	if (Vars::Aimbot::Mode == 1)
	{
		ImGui::SliderFloat("Smooth Amount", &Vars::Aimbot::SmoothAmount, 1.0f, 20.0f, "%.1f");
	}

	ImGui::SliderFloat("FOV", &Vars::Aimbot::FOV, 1.0f, 180.0f, "%.0f°");
	ImGui::Separator();

	// Target selection
	const char* targets[] = { "Distance", "FOV" };
	ImGui::Combo("Target Selection", &Vars::Aimbot::TargetSelection, targets, IM_ARRAYSIZE(targets));

	// Hitbox
	const char* hitboxes[] = { "Head", "Body", "Auto" };
	ImGui::Combo("Hitbox", &Vars::Aimbot::Hitbox, hitboxes, IM_ARRAYSIZE(hitboxes));

	ImGui::Separator();

	// Options
	ImGui::Checkbox("Auto Shoot", &Vars::Aimbot::AutoShoot);
	ImGui::Checkbox("Draw FOV Circle", &Vars::Aimbot::DrawFOV);
	ImGui::Checkbox("FFA Mode (Ignore Teams)", &Vars::Aimbot::FFAMode);
	ImGui::Checkbox("Ignore Cloaked", &Vars::Aimbot::IgnoreCloaked);
	ImGui::Checkbox("Ignore Invulnerable", &Vars::Aimbot::IgnoreInvulnerable);
	ImGui::Checkbox("Ignore Friends", &Vars::Aimbot::IgnoreFriends);

	ImGui::Separator();

	// Key bindings
	ImGui::Text("Aimbot Key: %s", GetKeyNameImGui(Vars::Aimbot::AimbotKey));
	if (m_bAimbotKeyListening)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
		ImGui::Text("PRESS ANY KEY...");
		ImGui::PopStyleColor();
	}
	else
	{
		if (ImGui::Button("Bind Aimbot Key"))
			m_bAimbotKeyListening = true;
	}

	ImGui::Separator();
}

void CImGuiMenu::DrawVisualsTab()
{
	ImGui::Text("ESP Settings");
	ImGui::Separator();
	
	ImGui::Checkbox("Enable ESP", &Vars::ESP::Enabled);
	
	ImGui::Separator();
	ImGui::Text("Players");
	ImGui::Checkbox("Player ESP", &Vars::ESP::Players);
	ImGui::Checkbox("Player Boxes", &Vars::ESP::PlayerBoxes);
	ImGui::Checkbox("Player Names", &Vars::ESP::PlayerNames);
	ImGui::Checkbox("Health Text", &Vars::ESP::PlayerHealth);
	ImGui::Checkbox("Health Bar", &Vars::ESP::PlayerHealthBar);
	ImGui::Checkbox("Player Weapons", &Vars::ESP::PlayerWeapons);
	ImGui::Checkbox("Player Conditions", &Vars::ESP::PlayerConditions);
	
	ImGui::Separator();
	ImGui::Text("World Items");
	ImGui::Checkbox("Item ESP", &Vars::ESP::Items);
	ImGui::Checkbox("Ammo Packs", &Vars::ESP::Ammo);
	ImGui::Checkbox("Health Packs", &Vars::ESP::Health);
	ImGui::Checkbox("Weapon Spawns", &Vars::ESP::Weapons);
	ImGui::Checkbox("Powerups", &Vars::ESP::Powerups);
	
	ImGui::Separator();
	ImGui::Checkbox("Show Keybind Window", &m_bShowKeybindWindow);
	ImGui::ColorEdit4("Window Color", m_fWindowColor);
}

void CImGuiMenu::DrawPlayersTab()
{
	ImGui::Text("Players in Game");
	ImGui::Separator();
	
	if (!I::EngineClient || !I::ClientEntityList || !I::EngineClient->IsConnected())
	{
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Not connected to a server");
		return;
	}
	
	ImGui::BeginChild("PlayerList", ImVec2(0, 0), true);
	
	int localPlayerIndex = I::EngineClient->GetLocalPlayer();
	
	for (int i = 1; i <= I::GlobalVarsBase->maxClients; i++)
	{
		// Skip local player
		if (i == localPlayerIndex)
			continue;
			
		auto pEntity = I::ClientEntityList->GetClientEntity(i);
		if (!pEntity)
			continue;
			
		C_TFPlayer* pPlayer = reinterpret_cast<C_TFPlayer*>(pEntity);
		
		player_info_t playerInfo;
		if (!I::EngineClient->GetPlayerInfo(i, &playerInfo))
			continue;
		
		// Get team color
		ImVec4 teamColor;
		int team = pPlayer->m_iTeamNum();
		if (team == 2) // RED
			teamColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
		else if (team == 3) // BLU
			teamColor = ImVec4(0.3f, 0.5f, 1.0f, 1.0f);
		else
			teamColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
		
		// Display player name with team color
		ImGui::TextColored(teamColor, "%s", playerInfo.name);
		ImGui::SameLine();
		
		// Friend checkbox (custom friends list)
		char checkboxLabel[64];
		sprintf_s(checkboxLabel, "Friend##%d", i);
		bool isFriend = Vars::CustomFriends.count(playerInfo.friendsID) > 0;
		if (ImGui::Checkbox(checkboxLabel, &isFriend))
		{
			if (isFriend)
				Vars::CustomFriends.insert(playerInfo.friendsID);
			else
				Vars::CustomFriends.erase(playerInfo.friendsID);
		}
	}
	
	ImGui::EndChild();
}

void CImGuiMenu::DrawMiscTab()
{
	ImGui::Text("Thirdperson");
	ImGui::Separator();
	
	ImGui::Checkbox("Enable Thirdperson", &Vars::Misc::Thirdperson);
	
	if (Vars::Misc::Thirdperson)
	{
		ImGui::SliderFloat("Right/Left", &Vars::Misc::ThirdpersonRight, -200.0f, 200.0f, "%.0f");
		ImGui::SliderFloat("Up/Down", &Vars::Misc::ThirdpersonUp, -200.0f, 200.0f, "%.0f");
		
		// Forward/Back slider - allow negative but display as 0 minimum
		float displayValue = Vars::Misc::ThirdpersonBack + 40.0f; // Offset for display
		if (ImGui::SliderFloat("Forward/Back", &displayValue, 0.0f, 340.0f, "%.0f"))
		{
			Vars::Misc::ThirdpersonBack = displayValue - 40.0f; // Store actual value with offset
		}
	}
}

void CImGuiMenu::DrawConfigTab()
{
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Configuration");
	ImGui::Separator();

	ImGui::Text("Config file: C:\\necromancer_OF2\\configs\\config.json");
	ImGui::Spacing();

	static bool saved = false;
	static bool loaded = false;
	static float timer = 0.0f;

	if (saved || loaded)
	{
		timer += ImGui::GetIO().DeltaTime;
		if (timer > 3.0f)
		{
			saved = false;
			loaded = false;
			timer = 0.0f;
		}
	}

	if (ImGui::Button("Save Config", ImVec2(150, 30)))
	{
		g_Config.SaveConfig();
		saved = true;
		timer = 0.0f;
	}

	ImGui::SameLine();

	if (ImGui::Button("Load Config", ImVec2(150, 30)))
	{
		g_Config.LoadConfig();
		loaded = true;
		timer = 0.0f;
	}

	if (saved)
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Config saved!");
	if (loaded)
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Config loaded!");
}

void CImGuiMenu::DrawAboutTab()
{
	ImGui::Text("Necromancer");
	ImGui::Text("Version 1.0");
	ImGui::Separator();

	ImGui::Text("Controls:");
	ImGui::BulletText("INSERT - Toggle menu");
	ImGui::Separator();

	ImGui::Text("Features:");
	ImGui::BulletText("ESP with customizable options");
	ImGui::BulletText("Config save/load");
}

void CImGuiMenu::DrawKeybindWindow()
{
	ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.6f);
	
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4 oldTitleBg = style.Colors[ImGuiCol_TitleBg];
	ImVec4 oldTitleBgActive = style.Colors[ImGuiCol_TitleBgActive];
	
	style.Colors[ImGuiCol_TitleBg] = ImVec4(m_fWindowColor[0], m_fWindowColor[1], m_fWindowColor[2], m_fWindowColor[3]);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(m_fWindowColor[0], m_fWindowColor[1], m_fWindowColor[2], m_fWindowColor[3]);

	ImGui::Begin("Keybinds", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

	// Aimbot status - three states
	bool aimbotKeyPressed = Vars::Aimbot::Enabled && (Vars::Aimbot::AimbotKey == 0 || (GetAsyncKeyState(Vars::Aimbot::AimbotKey) & 0x8000));
	bool aimbotTargeting = G::bAimbotActive;
	
	const char* statusText = nullptr;
	ImVec4 statusColor;
	
	if (!Vars::Aimbot::Enabled || !aimbotKeyPressed)
	{
		// Inactive - nothing happening
		statusText = "Inactive";
		statusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	}
	else if (aimbotKeyPressed && !aimbotTargeting)
	{
		// Idle - key pressed but not targeting anyone
		statusText = "Idle";
		statusColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		// Active - currently shooting and targeting someone
		statusText = "Active";
		statusColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	
	ImGui::Text("Aimbot");
	
	// Calculate text width and align to right
	float statusWidth = ImGui::CalcTextSize(statusText).x;
	ImGui::SameLine(ImGui::GetWindowWidth() - statusWidth - ImGui::GetStyle().WindowPadding.x);
	ImGui::TextColored(statusColor, statusText);

	ImGui::End();

	style.Colors[ImGuiCol_TitleBg] = oldTitleBg;
	style.Colors[ImGuiCol_TitleBgActive] = oldTitleBgActive;
}
