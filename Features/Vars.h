#pragma once

#include <Windows.h>
#include <unordered_set>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include "../SDK/Includes/basetypes.h"
#include "../vendor/nlohmann/json.hpp"

using json = nlohmann::json;

namespace Vars
{
	// Variable registration system
	struct VarEntry
	{
		std::string category;
		std::string name;
		std::function<void(json&)> save;
		std::function<void(const json&)> load;
	};
	
	inline std::vector<VarEntry> RegisteredVars;
	
	// Macro to register variables automatically
	#define REGISTER_VAR(cat, type, name, defaultVal) \
		inline type name = defaultVal; \
		inline bool _reg_##cat##_##name = []() { \
			Vars::RegisteredVars.push_back({ \
				#cat, \
				#name, \
				[](json& j) { j[#cat][#name] = Vars::cat::name; }, \
				[](const json& j) { if (j.contains(#cat) && j[#cat].contains(#name)) Vars::cat::name = j[#cat][#name]; } \
			}); \
			return true; \
		}();

	namespace Aimbot
	{
		REGISTER_VAR(Aimbot, bool, Enabled, false)
		REGISTER_VAR(Aimbot, int, Mode, 0) // 0 = Plain, 1 = Smooth, 2 = Silent
		REGISTER_VAR(Aimbot, float, SmoothAmount, 5.0f)
		REGISTER_VAR(Aimbot, float, FOV, 30.0f)
		REGISTER_VAR(Aimbot, int, TargetSelection, 1) // 0 = Distance, 1 = FOV, 2 = Health
		REGISTER_VAR(Aimbot, int, Hitbox, 2) // 0 = Head, 1 = Body, 2 = Auto (weapon-based)
		REGISTER_VAR(Aimbot, bool, IgnoreCloaked, true)
		REGISTER_VAR(Aimbot, bool, IgnoreInvulnerable, true)
		REGISTER_VAR(Aimbot, bool, AutoShoot, false)
		REGISTER_VAR(Aimbot, bool, DrawFOV, true)
		REGISTER_VAR(Aimbot, bool, FFAMode, true) // Free-for-all mode - ignores team check
		REGISTER_VAR(Aimbot, bool, IgnoreFriends, true) // Don't target marked friends
	}

	namespace ESP
	{
		REGISTER_VAR(ESP, bool, Enabled, true)
		REGISTER_VAR(ESP, bool, Players, true)
		REGISTER_VAR(ESP, bool, PlayerBoxes, true)
		REGISTER_VAR(ESP, bool, PlayerNames, true)
		REGISTER_VAR(ESP, bool, PlayerHealth, true)
		REGISTER_VAR(ESP, bool, PlayerHealthBar, true)
		REGISTER_VAR(ESP, bool, PlayerWeapons, true)
		REGISTER_VAR(ESP, bool, PlayerConditions, true)
		REGISTER_VAR(ESP, bool, Items, true)
		REGISTER_VAR(ESP, bool, Ammo, true)
		REGISTER_VAR(ESP, bool, Health, true)
		REGISTER_VAR(ESP, bool, Weapons, true)
		REGISTER_VAR(ESP, bool, Powerups, true)
	}

	namespace Misc
	{
		REGISTER_VAR(Misc, bool, Thirdperson, false)
		REGISTER_VAR(Misc, float, ThirdpersonRight, 0.0f)
		REGISTER_VAR(Misc, float, ThirdpersonUp, 0.0f)
		REGISTER_VAR(Misc, float, ThirdpersonBack, 150.0f)
		REGISTER_VAR(Misc, bool, ChatSpammer, false)
		REGISTER_VAR(Misc, float, ChatSpammerInterval, 2.0f)
		REGISTER_VAR(Misc, bool, ShowKeybindWindow, true)
		REGISTER_VAR(Misc, bool, AutoStrafe, false)
		REGISTER_VAR(Misc, float, AutoStrafeTurnRate, 0.5f)
		REGISTER_VAR(Misc, float, AutoStrafeMaxDelta, 90.0f)
	}
	
	namespace Colors
	{
		REGISTER_VAR(Colors, float, MenuBackgroundR, 0.05f)
		REGISTER_VAR(Colors, float, MenuBackgroundG, 0.05f)
		REGISTER_VAR(Colors, float, MenuBackgroundB, 0.05f)
		REGISTER_VAR(Colors, float, MenuBackgroundA, 0.95f)
		
		REGISTER_VAR(Colors, float, MenuTextR, 1.0f)
		REGISTER_VAR(Colors, float, MenuTextG, 1.0f)
		REGISTER_VAR(Colors, float, MenuTextB, 1.0f)
		REGISTER_VAR(Colors, float, MenuTextA, 1.0f)
		
		REGISTER_VAR(Colors, float, MenuAccentR, 0.0f)
		REGISTER_VAR(Colors, float, MenuAccentG, 0.86274f) // 220/255
		REGISTER_VAR(Colors, float, MenuAccentB, 0.0f)
		REGISTER_VAR(Colors, float, MenuAccentA, 1.0f)
		
		REGISTER_VAR(Colors, float, WindowColorR, 0.0f)
		REGISTER_VAR(Colors, float, WindowColorG, 0.86274f) // 220/255
		REGISTER_VAR(Colors, float, WindowColorB, 0.0f)
		REGISTER_VAR(Colors, float, WindowColorA, 1.0f)
		
		REGISTER_VAR(Colors, float, AimbotFOVR, 1.0f)
		REGISTER_VAR(Colors, float, AimbotFOVG, 1.0f)
		REGISTER_VAR(Colors, float, AimbotFOVB, 1.0f)
		REGISTER_VAR(Colors, float, AimbotFOVA, 0.4f)
	}
	
	// Custom friends list (Steam IDs)
	inline std::unordered_set<uint32> CustomFriends;

	// Bind system
	struct BindConfig
	{
		std::string name;
		int key = 0;
		bool isToggle = false; // false = hold, true = toggle
		bool isActive = false; // for toggle state
		
		// Stored settings for this bind
		std::unordered_map<std::string, bool> boolSettings;
		std::unordered_map<std::string, int> intSettings;
		std::unordered_map<std::string, float> floatSettings;
		
		// Previous settings (what was there before bind activated)
		std::unordered_map<std::string, bool> previousBoolSettings;
		std::unordered_map<std::string, int> previousIntSettings;
		std::unordered_map<std::string, float> previousFloatSettings;
	};

	inline std::vector<BindConfig> Binds;
}
