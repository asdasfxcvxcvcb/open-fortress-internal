#include "Menu.h"
#include "../Vars.h"
#include "../Aimbot/AimbotHitscan/AimbotHitscan.h"
#include "../ESP/ESP.h"
#include "../Config/Config.h"
#include "../Chat/Chat.h"
#include "../../vendor/imgui/imgui.h"
#include "../../vendor/imgui/imgui_impl_win32.h"
#include "../../vendor/imgui/imgui_impl_dx9.h"
#include <fstream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static CMenu* g_pMenuInstance = nullptr;

LRESULT CALLBACK CMenu::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!g_pMenuInstance)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	// Handle focus changes to reset input state
	if (uMsg == WM_KILLFOCUS || uMsg == WM_ACTIVATE)
	{
		// Reset ImGui input state when losing focus
		if (g_pMenuInstance->m_bInitialized)
		{
			ImGuiIO& io = ImGui::GetIO();
			// Clear mouse button states
			io.MouseDown[0] = false;
			io.MouseDown[1] = false;
			io.MouseDown[2] = false;
			io.MouseDown[3] = false;
			io.MouseDown[4] = false;
		}
		
		// If menu was open and we're losing focus, close it
		if (uMsg == WM_KILLFOCUS && g_pMenuInstance->m_bOpen.load())
		{
			g_pMenuInstance->m_bOpen.store(false);
			if (g_pMenuInstance->m_bInitialized)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseDrawCursor = false;
				io.WantCaptureMouse = false;
				io.WantCaptureKeyboard = false;
			}
		}
		
		// Always pass focus messages to the original handler
		return CallWindowProc(g_pMenuInstance->m_pOriginalWndProc, hWnd, uMsg, wParam, lParam);
	}

	// Toggle menu with INSERT key (handle before anything else)
	if (uMsg == WM_KEYDOWN && wParam == VK_INSERT)
	{
		g_pMenuInstance->Toggle();
		return 0;
	}

	// Block all mouse messages when menu is open
	if (g_pMenuInstance->m_bOpen.load())
	{
		// Handle bind key listening FIRST
		if (g_pMenuInstance->m_bBindKeyListening)
		{
			if (uMsg == WM_LBUTTONDOWN)
			{
				g_pMenuInstance->m_nTempBindKey = VK_LBUTTON;
				g_pMenuInstance->m_bBindKeyListening = false;
				return 1;
			}
			else if (uMsg == WM_RBUTTONDOWN)
			{
				g_pMenuInstance->m_nTempBindKey = VK_RBUTTON;
				g_pMenuInstance->m_bBindKeyListening = false;
				return 1;
			}
			else if (uMsg == WM_MBUTTONDOWN)
			{
				g_pMenuInstance->m_nTempBindKey = VK_MBUTTON;
				g_pMenuInstance->m_bBindKeyListening = false;
				return 1;
			}
			else if (uMsg == WM_XBUTTONDOWN)
			{
				int xButton = GET_XBUTTON_WPARAM(wParam);
				g_pMenuInstance->m_nTempBindKey = (xButton == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2;
				g_pMenuInstance->m_bBindKeyListening = false;
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
		if (g_pMenuInstance->m_bBindKeyListening)
		{
			// Capture key for bind system
			g_pMenuInstance->m_nTempBindKey = static_cast<int>(wParam);
			g_pMenuInstance->m_bBindKeyListening = false;
			return 0;
		}
	}

	// Call original window procedure
	return CallWindowProc(g_pMenuInstance->m_pOriginalWndProc, hWnd, uMsg, wParam, lParam);
}

bool CMenu::Initialize(IDirect3DDevice9* pDevice)
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
	style.Colors[ImGuiCol_CheckMark] = ImVec4(Vars::Colors::WindowColorR, Vars::Colors::WindowColorG, Vars::Colors::WindowColorB, Vars::Colors::WindowColorA);

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

void CMenu::Shutdown()
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

void CMenu::OnDeviceLost()
{
	if (m_bInitialized)
		ImGui_ImplDX9_InvalidateDeviceObjects();
}

void CMenu::OnDeviceReset()
{
	if (m_bInitialized)
		ImGui_ImplDX9_CreateDeviceObjects();
}

void CMenu::Toggle()
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
	case VK_NUMPAD0: return "Num 0";
	case VK_NUMPAD1: return "Num 1";
	case VK_NUMPAD2: return "Num 2";
	case VK_NUMPAD3: return "Num 3";
	case VK_NUMPAD4: return "Num 4";
	case VK_NUMPAD5: return "Num 5";
	case VK_NUMPAD6: return "Num 6";
	case VK_NUMPAD7: return "Num 7";
	case VK_NUMPAD8: return "Num 8";
	case VK_NUMPAD9: return "Num 9";
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

void CMenu::Render()
{
	if (!m_bInitialized)
		return;

	// Check bind activation (only when menu is closed)
	CheckBindActivation();

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

			// Draw the FOV circle with ImGui using custom color
			ImU32 circleColor = IM_COL32(
				static_cast<int>(Vars::Colors::AimbotFOVR * 255),
				static_cast<int>(Vars::Colors::AimbotFOVG * 255),
				static_cast<int>(Vars::Colors::AimbotFOVB * 255),
				static_cast<int>(Vars::Colors::AimbotFOVA * 255)
			);
			drawList->AddCircle(ImVec2(centerX, centerY), radius, circleColor, 64, 2.0f);
		}

		// Render ESP through ImGui DrawList (only if not using in-game render)
		if (!Vars::ESP::UseInGameRender)
			F::ESP.Render();
	}

	// Draw keybind window if enabled (in-game OR when menu is open)
	bool bShouldDrawKeybind = false;
	if (I::EngineClient && I::ClientEntityList && I::EngineClient->IsConnected())
	{
		auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
		if (pLocal && pLocal->As<C_TFPlayer*>()->m_iTeamNum() >= 2)
		{
			bShouldDrawKeybind = true;
		}
	}
	
	// Also show when menu is open so user can preview it
	if (m_bOpen.load())
		bShouldDrawKeybind = true;
	
	if (Vars::Misc::ShowKeybindWindow && bShouldDrawKeybind)
	{
		DrawKeybindWindow();
	}

	// Draw main menu if open
	if (m_bOpen.load())
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TitleBg] = ImVec4(Vars::Colors::WindowColorR, Vars::Colors::WindowColorG, Vars::Colors::WindowColorB, Vars::Colors::WindowColorA);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(Vars::Colors::WindowColorR, Vars::Colors::WindowColorG, Vars::Colors::WindowColorB, Vars::Colors::WindowColorA);

		ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
		bool bOpen = m_bOpen.load();
		ImGui::Begin("Necromancer", &bOpen, ImGuiWindowFlags_NoCollapse);
		if (!bOpen) m_bOpen.store(false);

		// Show editing indicator BEFORE tabs if in edit mode
		if (m_bInEditMode && m_nEditingBindIndex >= 0 && m_nEditingBindIndex < static_cast<int>(Vars::Binds.size()))
		{
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Editing Bind: %s", Vars::Binds[m_nEditingBindIndex].name.c_str());
			ImGui::SameLine(ImGui::GetWindowWidth() - 120);
			if (ImGui::Button("Done Editing", ImVec2(100, 0)))
			{
				// Save current settings to bind
				CaptureBindSettings(m_nEditingBindIndex);
				
				// Restore original settings using RegisteredVars
				for (const auto& var : Vars::RegisteredVars)
				{
					switch (var.type)
					{
					case Vars::VarEntry::Type::Bool:
						if (m_mOriginalBoolSettings.count(var.fullName))
							var.setBool(m_mOriginalBoolSettings[var.fullName]);
						break;
					case Vars::VarEntry::Type::Int:
						if (m_mOriginalIntSettings.count(var.fullName))
							var.setInt(m_mOriginalIntSettings[var.fullName]);
						break;
					case Vars::VarEntry::Type::Float:
						if (m_mOriginalFloatSettings.count(var.fullName))
							var.setFloat(m_mOriginalFloatSettings[var.fullName]);
						break;
					}
				}
				
				// Clear stored settings
				m_mOriginalBoolSettings.clear();
				m_mOriginalIntSettings.clear();
				m_mOriginalFloatSettings.clear();
				
				m_bInEditMode = false;
				m_nEditingBindIndex = -1;
			}
			ImGui::Separator();
		}

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

			// Only show Players tab when not in edit mode
			if (!m_bInEditMode)
			{
				if (ImGui::BeginTabItem("Players"))
				{
					DrawPlayersTab();
					ImGui::EndTabItem();
				}
			}

			if (ImGui::BeginTabItem("Misc"))
			{
				DrawMiscTab();
				ImGui::EndTabItem();
			}

			// Only show Binds and Config tabs when not in edit mode
			if (!m_bInEditMode)
			{
				if (ImGui::BeginTabItem("Binds"))
				{
					DrawBindsTab();
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Config"))
				{
					DrawConfigTab();
					ImGui::EndTabItem();
				}
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

void CMenu::DrawAimbotTab()
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
}

void CMenu::DrawVisualsTab()
{
	static int visualsSubTab = 0; // 0 = ESP, 1 = Color, 2 = Misc
	
	// Subcategory buttons
	if (ImGui::Button("ESP", ImVec2(100, 30)))
		visualsSubTab = 0;
	ImGui::SameLine();
	if (ImGui::Button("Color", ImVec2(100, 30)))
		visualsSubTab = 1;
	ImGui::SameLine();
	if (ImGui::Button("Misc", ImVec2(100, 30)))
		visualsSubTab = 2;
	
	ImGui::Separator();
	ImGui::Spacing();
	
	if (visualsSubTab == 0)
	{
		// ESP Settings - organized with columns
		ImGui::Columns(2, nullptr, false);
		
		// Left Column - Global & Players
		ImGui::BeginChild("ESPLeft", ImVec2(0, 0), true);
		{
			// Global Settings
			if (ImGui::CollapsingHeader("Global", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Checkbox("Enable ESP", &Vars::ESP::Enabled);
				ImGui::Checkbox("Use In-Game Render (smoother, lower FPS)", &Vars::ESP::UseInGameRender);
				ImGui::Spacing();
			}
			
			// Player ESP
			if (ImGui::CollapsingHeader("Players", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Checkbox("Player ESP", &Vars::ESP::Players);
				ImGui::Spacing();
				
				ImGui::Text("Filters:");
				ImGui::Indent();
				ImGui::Checkbox("Ignore Local", &Vars::ESP::IgnoreLocal);
				ImGui::Checkbox("Ignore Teammates", &Vars::ESP::IgnoreTeammates);
				ImGui::Checkbox("Ignore Enemies", &Vars::ESP::IgnoreEnemies);
				ImGui::Checkbox("Ignore Friends", &Vars::ESP::IgnoreFriends);
				ImGui::Checkbox("Ignore Cloaked", &Vars::ESP::IgnoreCloaked);
				ImGui::Unindent();
				ImGui::Spacing();
				
				ImGui::Text("Draw:");
				ImGui::Indent();
				ImGui::Checkbox("Boxes", &Vars::ESP::PlayerBoxes);
				ImGui::Checkbox("Names", &Vars::ESP::PlayerNames);
				ImGui::Checkbox("Health Text", &Vars::ESP::PlayerHealth);
				ImGui::Checkbox("Health Bar", &Vars::ESP::PlayerHealthBar);
				ImGui::Checkbox("Weapons", &Vars::ESP::PlayerWeapons);
				ImGui::Checkbox("Conditions", &Vars::ESP::PlayerConditions);
				ImGui::Checkbox("Skeleton", &Vars::ESP::PlayerSkeleton);
				ImGui::Unindent();
			}
		}
		ImGui::EndChild();
		
		ImGui::NextColumn();
		
		// Right Column - World Items
		ImGui::BeginChild("ESPRight", ImVec2(0, 0), true);
		{
			if (ImGui::CollapsingHeader("World Items", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Checkbox("Item ESP", &Vars::ESP::Items);
				ImGui::Spacing();
				
				ImGui::Text("Draw:");
				ImGui::Indent();
				ImGui::Checkbox("Ammo Packs", &Vars::ESP::Ammo);
				ImGui::Checkbox("Health Packs", &Vars::ESP::Health);
				ImGui::Checkbox("Weapon Spawns", &Vars::ESP::Weapons);
				ImGui::Checkbox("Powerups", &Vars::ESP::Powerups);
				ImGui::Unindent();
			}
		}
		ImGui::EndChild();
		
		ImGui::Columns(1);
	}
	else if (visualsSubTab == 1)
	{
		// Color Settings
		ImGui::Text("Color Customization");
		ImGui::Separator();
		
		// Auto-apply colors when changed
		ImGuiStyle& style = ImGui::GetStyle();
		
		// Color picker flags: show picker popup, alpha bar separate, smaller width
		ImGuiColorEditFlags colorFlags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_PickerHueWheel;
		
		// Menu Colors
		ImGui::Text("Menu");
		float menuBg[4] = { Vars::Colors::MenuBackgroundR, Vars::Colors::MenuBackgroundG, Vars::Colors::MenuBackgroundB, Vars::Colors::MenuBackgroundA };
		if (ImGui::ColorEdit4("Menu Background", menuBg, colorFlags))
		{
			Vars::Colors::MenuBackgroundR = menuBg[0]; Vars::Colors::MenuBackgroundG = menuBg[1]; Vars::Colors::MenuBackgroundB = menuBg[2]; Vars::Colors::MenuBackgroundA = menuBg[3];
			style.Colors[ImGuiCol_WindowBg] = ImVec4(menuBg[0], menuBg[1], menuBg[2], menuBg[3]);
		}
		
		float menuText[4] = { Vars::Colors::MenuTextR, Vars::Colors::MenuTextG, Vars::Colors::MenuTextB, Vars::Colors::MenuTextA };
		if (ImGui::ColorEdit4("Menu Text", menuText, colorFlags))
		{
			Vars::Colors::MenuTextR = menuText[0]; Vars::Colors::MenuTextG = menuText[1]; Vars::Colors::MenuTextB = menuText[2]; Vars::Colors::MenuTextA = menuText[3];
			style.Colors[ImGuiCol_Text] = ImVec4(menuText[0], menuText[1], menuText[2], menuText[3]);
		}
		
		float menuAccent[4] = { Vars::Colors::MenuAccentR, Vars::Colors::MenuAccentG, Vars::Colors::MenuAccentB, Vars::Colors::MenuAccentA };
		if (ImGui::ColorEdit4("Menu Accent", menuAccent, colorFlags))
		{
			Vars::Colors::MenuAccentR = menuAccent[0]; Vars::Colors::MenuAccentG = menuAccent[1]; Vars::Colors::MenuAccentB = menuAccent[2]; Vars::Colors::MenuAccentA = menuAccent[3];
			style.Colors[ImGuiCol_CheckMark] = ImVec4(menuAccent[0], menuAccent[1], menuAccent[2], menuAccent[3]);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(menuAccent[0], menuAccent[1], menuAccent[2], menuAccent[3]);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(menuAccent[0], menuAccent[1], menuAccent[2], 1.0f);
			style.Colors[ImGuiCol_Button] = ImVec4(menuAccent[0], menuAccent[1], menuAccent[2], 0.4f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(menuAccent[0], menuAccent[1], menuAccent[2], 0.6f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(menuAccent[0], menuAccent[1], menuAccent[2], 0.8f);
			style.Colors[ImGuiCol_Header] = ImVec4(menuAccent[0], menuAccent[1], menuAccent[2], 0.4f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(menuAccent[0], menuAccent[1], menuAccent[2], 0.6f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(menuAccent[0], menuAccent[1], menuAccent[2], 0.8f);
		}
		
		float windowColor[4] = { Vars::Colors::WindowColorR, Vars::Colors::WindowColorG, Vars::Colors::WindowColorB, Vars::Colors::WindowColorA };
		if (ImGui::ColorEdit4("Keybind Window", windowColor, colorFlags))
		{
			Vars::Colors::WindowColorR = windowColor[0]; Vars::Colors::WindowColorG = windowColor[1]; Vars::Colors::WindowColorB = windowColor[2]; Vars::Colors::WindowColorA = windowColor[3];
		}
		
		ImGui::Separator();
		
		// Aimbot Colors
		ImGui::Text("Aimbot");
		float fovColor[4] = { Vars::Colors::AimbotFOVR, Vars::Colors::AimbotFOVG, Vars::Colors::AimbotFOVB, Vars::Colors::AimbotFOVA };
		if (ImGui::ColorEdit4("FOV Circle", fovColor, colorFlags))
		{
			Vars::Colors::AimbotFOVR = fovColor[0]; Vars::Colors::AimbotFOVG = fovColor[1]; Vars::Colors::AimbotFOVB = fovColor[2]; Vars::Colors::AimbotFOVA = fovColor[3];
		}
		
		ImGui::Separator();
		
		// ESP Colors
		ImGui::Text("ESP");
		ImGui::Checkbox("Use Enemy Colors", &Vars::ESP::UseEnemyColors);
		ImGui::Spacing();
		
		float boxColor[4] = { Vars::Colors::ESPBoxR, Vars::Colors::ESPBoxG, Vars::Colors::ESPBoxB, Vars::Colors::ESPBoxA };
		if (ImGui::ColorEdit4("Box", boxColor, colorFlags))
		{
			Vars::Colors::ESPBoxR = boxColor[0]; Vars::Colors::ESPBoxG = boxColor[1]; Vars::Colors::ESPBoxB = boxColor[2]; Vars::Colors::ESPBoxA = boxColor[3];
		}
		
		float skeletonColor[4] = { Vars::Colors::SkeletonR, Vars::Colors::SkeletonG, Vars::Colors::SkeletonB, Vars::Colors::SkeletonA };
		if (ImGui::ColorEdit4("Skeleton", skeletonColor, colorFlags))
		{
			Vars::Colors::SkeletonR = skeletonColor[0]; Vars::Colors::SkeletonG = skeletonColor[1]; Vars::Colors::SkeletonB = skeletonColor[2]; Vars::Colors::SkeletonA = skeletonColor[3];
		}
		
		ImGui::Separator();
		
		// Reset button
		if (ImGui::Button("Reset to Default", ImVec2(200, 30)))
		{
			// Reset to defaults
			Vars::Colors::MenuBackgroundR = 0.05f; Vars::Colors::MenuBackgroundG = 0.05f; Vars::Colors::MenuBackgroundB = 0.05f; Vars::Colors::MenuBackgroundA = 0.95f;
			Vars::Colors::MenuTextR = 1.0f; Vars::Colors::MenuTextG = 1.0f; Vars::Colors::MenuTextB = 1.0f; Vars::Colors::MenuTextA = 1.0f;
			Vars::Colors::MenuAccentR = 0.0f; Vars::Colors::MenuAccentG = 0.86274f; Vars::Colors::MenuAccentB = 0.0f; Vars::Colors::MenuAccentA = 1.0f;
			Vars::Colors::WindowColorR = 0.0f; Vars::Colors::WindowColorG = 0.86274f; Vars::Colors::WindowColorB = 0.0f; Vars::Colors::WindowColorA = 1.0f;
			Vars::Colors::AimbotFOVR = 1.0f; Vars::Colors::AimbotFOVG = 1.0f; Vars::Colors::AimbotFOVB = 1.0f; Vars::Colors::AimbotFOVA = 0.4f;
			Vars::Colors::ESPBoxR = 1.0f; Vars::Colors::ESPBoxG = 1.0f; Vars::Colors::ESPBoxB = 1.0f; Vars::Colors::ESPBoxA = 1.0f;
			Vars::Colors::SkeletonR = 1.0f; Vars::Colors::SkeletonG = 1.0f; Vars::Colors::SkeletonB = 1.0f; Vars::Colors::SkeletonA = 1.0f;
			
			// Apply defaults
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.95f);
			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.86274f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.86274f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.86274f, 0.0f, 1.0f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.86274f, 0.0f, 0.4f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.86274f, 0.0f, 0.6f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.86274f, 0.0f, 0.8f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.86274f, 0.0f, 0.4f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.86274f, 0.0f, 0.6f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.86274f, 0.0f, 0.8f);
		}
	}
	else if (visualsSubTab == 2)
	{
		// Misc Visual Settings (thirdperson, etc.)
		ImGui::Text("Visual Misc");
		ImGui::Separator();
		
		ImGui::Checkbox("Show Keybind Window", &Vars::Misc::ShowKeybindWindow);
		
		ImGui::Separator();
		ImGui::Text("Thirdperson");
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
}

void CMenu::DrawPlayersTab()
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
		
		// Skip bots or invalid players (friendsID of 0 usually means bot)
		if (playerInfo.friendsID == 0 || playerInfo.fakeplayer)
		{
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s [BOT]", playerInfo.name);
			continue;
		}
		
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
		sprintf_s(checkboxLabel, "Friend##%d_%u", i, playerInfo.friendsID);
		bool isFriend = Vars::CustomFriends.count(playerInfo.friendsID) > 0;
		if (ImGui::Checkbox(checkboxLabel, &isFriend))
		{
			if (isFriend)
				Vars::CustomFriends.insert(playerInfo.friendsID);
			else
				Vars::CustomFriends.erase(playerInfo.friendsID);
				
			// Save to playerlist.json
			g_Config.SavePlayerList();
		}
	}
	
	ImGui::EndChild();
}

void CMenu::DrawMiscTab()
{
	static int miscSubTab = 0; // 0 = Movement, 1 = Chat
	
	// Subcategory buttons
	if (ImGui::Button("Movement", ImVec2(100, 30)))
		miscSubTab = 0;
	ImGui::SameLine();
	if (ImGui::Button("Chat", ImVec2(100, 30)))
		miscSubTab = 1;
	
	ImGui::Separator();
	ImGui::Spacing();
	
	if (miscSubTab == 0)
	{
		// Movement Settings
		ImGui::Text("Auto Strafe");
		ImGui::Separator();
		
		ImGui::Checkbox("Enable Auto Strafe", &Vars::Misc::AutoStrafe);
		
		if (Vars::Misc::AutoStrafe)
		{
			ImGui::PushItemWidth(200);
			ImGui::SliderFloat("Turn Rate", &Vars::Misc::AutoStrafeTurnRate, 0.1f, 1.0f, "%.2f");
			ImGui::SliderFloat("Max Delta", &Vars::Misc::AutoStrafeMaxDelta, 10.0f, 180.0f, "%.0f°");
			ImGui::PopItemWidth();
		}
	}
	else if (miscSubTab == 1)
	{
		// Chat Settings
		ImGui::Text("Chat Spammer");
		ImGui::Separator();
		
		ImGui::Checkbox("Enable Chat Spammer", &Vars::Misc::ChatSpammer);
		
		if (Vars::Misc::ChatSpammer)
		{
			ImGui::PushItemWidth(200);
			ImGui::SliderFloat("Interval", &Vars::Misc::ChatSpammerInterval, 0.1f, 10.0f, "%.1fs");
			ImGui::PopItemWidth();
			
			if (ImGui::Button("Refresh", ImVec2(100, 30)))
			{
				F::Chat.RefreshMessages();
			}
			
			ImGui::SameLine();
			
			if (ImGui::Button("Open Folder", ImVec2(100, 30)))
			{
				ShellExecuteA(NULL, "open", "C:\\necromancer_OF2\\chat", NULL, NULL, SW_SHOWNORMAL);
			}
		}
	}
}

void CMenu::CaptureBindSettings(int bindIndex)
{
	if (bindIndex < 0 || bindIndex >= static_cast<int>(Vars::Binds.size()))
		return;

	auto& bind = Vars::Binds[bindIndex];
	bind.boolSettings.clear();
	bind.intSettings.clear();
	bind.floatSettings.clear();

	// Automatically capture all changed settings using RegisteredVars
	for (const auto& var : Vars::RegisteredVars)
	{
		switch (var.type)
		{
		case Vars::VarEntry::Type::Bool:
		{
			if (m_mOriginalBoolSettings.count(var.fullName))
			{
				bool currentValue = var.getBool();
				if (currentValue != m_mOriginalBoolSettings[var.fullName])
					bind.boolSettings[var.fullName] = currentValue;
			}
			break;
		}
		case Vars::VarEntry::Type::Int:
		{
			if (m_mOriginalIntSettings.count(var.fullName))
			{
				int currentValue = var.getInt();
				if (currentValue != m_mOriginalIntSettings[var.fullName])
					bind.intSettings[var.fullName] = currentValue;
			}
			break;
		}
		case Vars::VarEntry::Type::Float:
		{
			if (m_mOriginalFloatSettings.count(var.fullName))
			{
				float currentValue = var.getFloat();
				if (currentValue != m_mOriginalFloatSettings[var.fullName])
					bind.floatSettings[var.fullName] = currentValue;
			}
			break;
		}
		}
	}
}

void CMenu::ApplyBindSettings(int bindIndex)
{
	if (bindIndex < 0 || bindIndex >= static_cast<int>(Vars::Binds.size()))
		return;

	auto& bind = Vars::Binds[bindIndex];

	// Automatically apply all settings using RegisteredVars
	for (const auto& var : Vars::RegisteredVars)
	{
		switch (var.type)
		{
		case Vars::VarEntry::Type::Bool:
			if (bind.boolSettings.count(var.fullName))
				var.setBool(bind.boolSettings[var.fullName]);
			break;
		case Vars::VarEntry::Type::Int:
			if (bind.intSettings.count(var.fullName))
				var.setInt(bind.intSettings[var.fullName]);
			break;
		case Vars::VarEntry::Type::Float:
			if (bind.floatSettings.count(var.fullName))
				var.setFloat(bind.floatSettings[var.fullName]);
			break;
		}
	}
}

void CMenu::CheckBindActivation()
{
	// Don't check binds when menu is open
	if (m_bOpen.load())
		return;

	// Process each bind independently
	for (size_t i = 0; i < Vars::Binds.size(); i++)
	{
		auto& bind = Vars::Binds[i];
		if (bind.key == 0)
			continue;

		bool keyPressed = (GetAsyncKeyState(bind.key) & 0x8000) != 0;
		bool wasActive = bind.isActive;

		// Update activation state
		if (bind.isToggle)
		{
			// Toggle mode - flip on key press
			static bool lastKeyState[256] = { false };
			int keyIndex = bind.key % 256;

			if (keyPressed && !lastKeyState[keyIndex])
			{
				bind.isActive = !bind.isActive;
			}
			lastKeyState[keyIndex] = keyPressed;
		}
		else
		{
			// Hold mode - active while key is held
			bind.isActive = keyPressed;
		}

		// Handle state changes
		if (bind.isActive && !wasActive)
		{
			// Bind just activated - store current values using RegisteredVars
			for (const auto& var : Vars::RegisteredVars)
			{
				switch (var.type)
				{
				case Vars::VarEntry::Type::Bool:
					if (bind.boolSettings.count(var.fullName))
						bind.previousBoolSettings[var.fullName] = var.getBool();
					break;
				case Vars::VarEntry::Type::Int:
					if (bind.intSettings.count(var.fullName))
						bind.previousIntSettings[var.fullName] = var.getInt();
					break;
				case Vars::VarEntry::Type::Float:
					if (bind.floatSettings.count(var.fullName))
						bind.previousFloatSettings[var.fullName] = var.getFloat();
					break;
				}
			}

			// Apply the bind settings
			ApplyBindSettings(static_cast<int>(i));
		}
		else if (!bind.isActive && wasActive)
		{
			// Bind just deactivated - restore previous values using RegisteredVars
			for (const auto& var : Vars::RegisteredVars)
			{
				switch (var.type)
				{
				case Vars::VarEntry::Type::Bool:
					if (bind.previousBoolSettings.count(var.fullName))
						var.setBool(bind.previousBoolSettings[var.fullName]);
					break;
				case Vars::VarEntry::Type::Int:
					if (bind.previousIntSettings.count(var.fullName))
						var.setInt(bind.previousIntSettings[var.fullName]);
					break;
				case Vars::VarEntry::Type::Float:
					if (bind.previousFloatSettings.count(var.fullName))
						var.setFloat(bind.previousFloatSettings[var.fullName]);
					break;
				}
			}

			// Clear previous settings
			bind.previousBoolSettings.clear();
			bind.previousIntSettings.clear();
			bind.previousFloatSettings.clear();
		}
	}
}

void CMenu::DrawBindsTab()
{
	// Normal bind creation UI
	ImGui::Text("Create New Bind");
	ImGui::Separator();

	// Top section: Name, Bind, Type, + button
	float itemWidth = 120.0f;
	float spacing = 10.0f;

	// Name input
	ImGui::Text("name");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	if (ImGui::InputText("##BindName", m_szBindName, 11))
	{
		// Limit to 10 characters
		if (strlen(m_szBindName) > 10)
			m_szBindName[10] = '\0';
	}

	ImGui::SameLine(0, spacing);

	// Bind input
	ImGui::Text("bind");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	if (m_bBindKeyListening && m_nEditingBindPropsIndex == -1)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
		ImGui::Button("PRESS KEY...", ImVec2(itemWidth, 0));
		ImGui::PopStyleColor();
	}
	else
	{
		if (ImGui::Button(m_nTempBindKey == 0 ? "Click to bind" : GetKeyNameImGui(m_nTempBindKey), ImVec2(itemWidth, 0)))
		{
			m_bBindKeyListening = true;
		}
	}

	ImGui::SameLine(0, spacing);

	// Type dropdown
	ImGui::Text("type");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(itemWidth);
	const char* types[] = { "hold", "toggle" };
	ImGui::Combo("##BindType", &m_nBindType, types, 2);

	ImGui::SameLine(0, spacing);

	// + button (faded if no name or key)
	bool hasName = strlen(m_szBindName) > 0;
	bool hasKey = m_nTempBindKey != 0;
	bool canCreate = hasName && hasKey;

	if (!canCreate)
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.3f);

	if (ImGui::Button("+", ImVec2(30, 30)) && canCreate)
	{
		// Create new bind
		Vars::BindConfig newBind;
		newBind.name = m_szBindName;
		newBind.key = m_nTempBindKey;
		newBind.isToggle = (m_nBindType == 1);
		Vars::Binds.push_back(newBind);

		// Clear input
		m_szBindName[0] = '\0';
		m_nTempBindKey = 0;
		m_nBindType = 0;
	}

	if (!canCreate)
		ImGui::PopStyleVar();

	ImGui::Separator();
	ImGui::Spacing();

	// List of created binds
	ImGui::Text("Binds");
	ImGui::Separator();

	if (Vars::Binds.empty())
	{
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No binds created");
	}
	else
	{
		ImGui::BeginChild("BindsList", ImVec2(0, 0), true);

		for (size_t i = 0; i < Vars::Binds.size(); i++)
		{
			auto& bind = Vars::Binds[i];
			int idx = static_cast<int>(i);

			ImGui::PushID(idx);

			// Bind rectangle
			ImVec2 cursorPos = ImGui::GetCursorScreenPos();
			ImVec2 rectSize = ImVec2(ImGui::GetContentRegionAvail().x, 40);

			ImDrawList* drawList = ImGui::GetWindowDrawList();

			// Highlight if editing properties
			ImU32 bgColor = (m_nEditingBindPropsIndex == idx) ? IM_COL32(50, 50, 50, 255) : IM_COL32(30, 30, 30, 255);
			drawList->AddRectFilled(cursorPos, ImVec2(cursorPos.x + rectSize.x, cursorPos.y + rectSize.y), bgColor);
			drawList->AddRect(cursorPos, ImVec2(cursorPos.x + rectSize.x, cursorPos.y + rectSize.y),
				IM_COL32(60, 60, 60, 255));

			// Make rectangle clickable to edit bind properties
			ImGui::SetCursorScreenPos(cursorPos);
			ImGui::InvisibleButton(("##BindRect" + std::to_string(idx)).c_str(), ImVec2(rectSize.x - 160, rectSize.y));
			if (ImGui::IsItemClicked())
			{
				if (m_nEditingBindPropsIndex == idx)
				{
					// Close if already editing
					m_nEditingBindPropsIndex = -1;
					m_nTempBindKey = 0;
				}
				else
				{
					// Open for editing
					m_nEditingBindPropsIndex = idx;
					m_nTempBindKey = 0;
				}
			}

			// Name on left
			ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + 10, cursorPos.y + 10));
			ImGui::Text("%s [%s] (%s)", bind.name.c_str(), GetKeyNameImGui(bind.key), bind.isToggle ? "toggle" : "hold");

			// Edit and Delete buttons on right
			ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + rectSize.x - 150, cursorPos.y + 5));
			if (ImGui::Button("Edit", ImVec2(60, 30)))
			{
				// Load bind settings into vars first
				ApplyBindSettings(idx);
				
				// THEN store these as the original baseline for comparison using RegisteredVars
				// This way we only capture what changes from the bind's current state
				m_mOriginalBoolSettings.clear();
				m_mOriginalIntSettings.clear();
				m_mOriginalFloatSettings.clear();
				
				for (const auto& var : Vars::RegisteredVars)
				{
					switch (var.type)
					{
					case Vars::VarEntry::Type::Bool:
						m_mOriginalBoolSettings[var.fullName] = var.getBool();
						break;
					case Vars::VarEntry::Type::Int:
						m_mOriginalIntSettings[var.fullName] = var.getInt();
						break;
					case Vars::VarEntry::Type::Float:
						m_mOriginalFloatSettings[var.fullName] = var.getFloat();
						break;
					}
				}

				// Enter edit mode
				m_bInEditMode = true;
				m_nEditingBindIndex = idx;
			}

			ImGui::SameLine();
			if (ImGui::Button("Delete", ImVec2(70, 30)))
			{
				Vars::Binds.erase(Vars::Binds.begin() + i);
				if (m_nEditingBindPropsIndex == idx)
					m_nEditingBindPropsIndex = -1;
				ImGui::PopID();
				break;
			}

			// Show edit controls below if this bind is being edited
			if (m_nEditingBindPropsIndex == idx)
			{
				ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + rectSize.y + 5));

				// Edit controls
				ImGui::BeginGroup();
				ImGui::Indent(10);

				ImGui::Text("Key:");
				ImGui::SameLine();
				if (m_bBindKeyListening)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
					ImGui::Button("PRESS KEY...", ImVec2(120, 0));
					ImGui::PopStyleColor();
				}
				else
				{
					if (ImGui::Button(GetKeyNameImGui(bind.key), ImVec2(120, 0)))
					{
						m_bBindKeyListening = true;
					}
				}

				// Update bind key if listening stopped
				if (!m_bBindKeyListening && m_nTempBindKey != 0 && m_nTempBindKey != bind.key)
				{
					bind.key = m_nTempBindKey;
					m_nTempBindKey = 0;
				}

				ImGui::SameLine();
				ImGui::Text("Type:");
				ImGui::SameLine();
				int currentType = bind.isToggle ? 1 : 0;
				ImGui::SetNextItemWidth(120);
				if (ImGui::Combo("##EditType", &currentType, types, 2))
				{
					bind.isToggle = (currentType == 1);
				}

				ImGui::Unindent(10);
				ImGui::EndGroup();

				ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + rectSize.y + 40));
			}
			else
			{
				ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + rectSize.y + 5));
			}

			ImGui::PopID();
		}

		ImGui::EndChild();
	}
}

void CMenu::DrawConfigTab()
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

void CMenu::DrawKeybindWindow()
{
	ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.6f);
	
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4 oldTitleBg = style.Colors[ImGuiCol_TitleBg];
	ImVec4 oldTitleBgActive = style.Colors[ImGuiCol_TitleBgActive];
	
	style.Colors[ImGuiCol_TitleBg] = ImVec4(Vars::Colors::WindowColorR, Vars::Colors::WindowColorG, Vars::Colors::WindowColorB, Vars::Colors::WindowColorA);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(Vars::Colors::WindowColorR, Vars::Colors::WindowColorG, Vars::Colors::WindowColorB, Vars::Colors::WindowColorA);

	ImGui::Begin("Keybinds", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

	// Aimbot status - three states
	bool aimbotEnabled = Vars::Aimbot::Enabled;
	bool aimbotTargeting = G::bAimbotActive;
	
	const char* statusText = nullptr;
	ImVec4 statusColor;
	
	if (!aimbotEnabled)
	{
		// Inactive - nothing happening
		statusText = "Inactive";
		statusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	}
	else if (aimbotEnabled && !aimbotTargeting)
	{
		// Idle - enabled but not targeting anyone
		statusText = "Idle";
		statusColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		// Active - currently targeting someone
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
