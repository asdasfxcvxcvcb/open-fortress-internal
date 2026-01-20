#pragma once

#include <Windows.h>
#include <unordered_set>
#include <vector>
#include <string>
#include <unordered_map>
#include "../SDK/Includes/basetypes.h"

namespace Vars
{
	namespace Aimbot
	{
		inline bool Enabled = false;
		inline int Mode = 0; // 0 = Plain, 1 = Smooth, 2 = Silent
		inline float SmoothAmount = 5.0f;
		inline float FOV = 30.0f;
		inline int TargetSelection = 1; // 0 = Distance, 1 = FOV, 2 = Health
		inline int Hitbox = 2; // 0 = Head, 1 = Body, 2 = Auto (weapon-based)
		inline bool IgnoreCloaked = true;
		inline bool IgnoreInvulnerable = true;
		inline bool AutoShoot = false;
		inline bool DrawFOV = true;
		inline bool FFAMode = true; // Free-for-all mode - ignores team check
		inline bool IgnoreFriends = true; // Don't target marked friends
	}

	namespace ESP
	{
		inline bool Enabled = true;
		inline bool Players = true;
		inline bool PlayerBoxes = true;
		inline bool PlayerNames = true;
		inline bool PlayerHealth = true;
		inline bool PlayerHealthBar = true;
		inline bool PlayerWeapons = true;
		inline bool PlayerConditions = true;
		inline bool Items = true;
		inline bool Ammo = true;
		inline bool Health = true;
		inline bool Weapons = true;
		inline bool Powerups = true;
	}

	namespace Misc
	{
		inline bool Thirdperson = false;
		inline float ThirdpersonRight = 0.0f;
		inline float ThirdpersonUp = 0.0f;
		inline float ThirdpersonBack = 150.0f;
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