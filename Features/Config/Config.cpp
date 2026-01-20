#include "Config.h"
#include "../Vars.h"
#include "../../SDK/SDK.h"
#include "../../vendor/nlohmann/json.hpp"
#include <fstream>
#include <Windows.h>

using json = nlohmann::json;

std::string CConfig::GetConfigPath(const std::string& configName)
{
	return "C:\\necromancer_OF2\\configs\\" + configName + ".json";
}

void CConfig::CreateDirectoryIfNotExists(const std::string& path)
{
	size_t lastSlash = path.find_last_of("\\/");
	if (lastSlash != std::string::npos)
	{
		std::string dir = path.substr(0, lastSlash);
		
		std::string currentPath;
		size_t pos = 0;
		while ((pos = dir.find('\\', pos)) != std::string::npos)
		{
			currentPath = dir.substr(0, pos);
			if (!currentPath.empty() && currentPath.back() != ':')
				CreateDirectoryA(currentPath.c_str(), NULL);
			pos++;
		}
		CreateDirectoryA(dir.c_str(), NULL);
	}
}

void CConfig::SaveConfig(const std::string& configName)
{
	std::string path = GetConfigPath(configName);
	CreateDirectoryIfNotExists(path);
	
	json config;
	
	// Aimbot settings
	config["Aimbot"]["Enabled"] = Vars::Aimbot::Enabled;
	config["Aimbot"]["Mode"] = Vars::Aimbot::Mode;
	config["Aimbot"]["SmoothAmount"] = Vars::Aimbot::SmoothAmount;
	config["Aimbot"]["FOV"] = Vars::Aimbot::FOV;
	config["Aimbot"]["TargetSelection"] = Vars::Aimbot::TargetSelection;
	config["Aimbot"]["Hitbox"] = Vars::Aimbot::Hitbox;
	config["Aimbot"]["IgnoreCloaked"] = Vars::Aimbot::IgnoreCloaked;
	config["Aimbot"]["IgnoreInvulnerable"] = Vars::Aimbot::IgnoreInvulnerable;
	config["Aimbot"]["AimbotKey"] = Vars::Aimbot::AimbotKey;
	config["Aimbot"]["AutoShoot"] = Vars::Aimbot::AutoShoot;
	config["Aimbot"]["DrawFOV"] = Vars::Aimbot::DrawFOV;
	config["Aimbot"]["FFAMode"] = Vars::Aimbot::FFAMode;
	config["Aimbot"]["IgnoreFriends"] = Vars::Aimbot::IgnoreFriends;
	
	// ESP settings
	config["ESP"]["Enabled"] = Vars::ESP::Enabled;
	config["ESP"]["Players"] = Vars::ESP::Players;
	config["ESP"]["PlayerBoxes"] = Vars::ESP::PlayerBoxes;
	config["ESP"]["PlayerNames"] = Vars::ESP::PlayerNames;
	config["ESP"]["PlayerHealth"] = Vars::ESP::PlayerHealth;
	config["ESP"]["PlayerHealthBar"] = Vars::ESP::PlayerHealthBar;
	config["ESP"]["PlayerWeapons"] = Vars::ESP::PlayerWeapons;
	config["ESP"]["PlayerConditions"] = Vars::ESP::PlayerConditions;
	config["ESP"]["Items"] = Vars::ESP::Items;
	config["ESP"]["Ammo"] = Vars::ESP::Ammo;
	config["ESP"]["Health"] = Vars::ESP::Health;
	config["ESP"]["Weapons"] = Vars::ESP::Weapons;
	config["ESP"]["Powerups"] = Vars::ESP::Powerups;
	
	// Misc settings
	config["Misc"]["Thirdperson"] = Vars::Misc::Thirdperson;
	config["Misc"]["ThirdpersonRight"] = Vars::Misc::ThirdpersonRight;
	config["Misc"]["ThirdpersonUp"] = Vars::Misc::ThirdpersonUp;
	config["Misc"]["ThirdpersonBack"] = Vars::Misc::ThirdpersonBack;
	
	std::ofstream file(path);
	if (file.is_open())
	{
		file << config.dump(4); // Pretty print with 4 space indent
		file.close();
		
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[Config] Saved to: %s\n", path.c_str());
	}
	else if (I::Cvar)
	{
		I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[Config] Failed to save: %s\n", path.c_str());
	}
}

void CConfig::LoadConfig(const std::string& configName)
{
	std::string path = GetConfigPath(configName);
	std::ifstream file(path);
	
	if (!file.is_open())
		return;
	
	try
	{
		json config;
		file >> config;
		file.close();
		
		// Aimbot settings
		if (config.contains("Aimbot"))
		{
			auto& aimbot = config["Aimbot"];
			if (aimbot.contains("Enabled")) Vars::Aimbot::Enabled = aimbot["Enabled"];
			if (aimbot.contains("Mode")) Vars::Aimbot::Mode = aimbot["Mode"];
			if (aimbot.contains("SmoothAmount")) Vars::Aimbot::SmoothAmount = aimbot["SmoothAmount"];
			if (aimbot.contains("FOV")) Vars::Aimbot::FOV = aimbot["FOV"];
			if (aimbot.contains("TargetSelection")) Vars::Aimbot::TargetSelection = aimbot["TargetSelection"];
			if (aimbot.contains("Hitbox")) Vars::Aimbot::Hitbox = aimbot["Hitbox"];
			if (aimbot.contains("IgnoreCloaked")) Vars::Aimbot::IgnoreCloaked = aimbot["IgnoreCloaked"];
			if (aimbot.contains("IgnoreInvulnerable")) Vars::Aimbot::IgnoreInvulnerable = aimbot["IgnoreInvulnerable"];
			if (aimbot.contains("AimbotKey")) Vars::Aimbot::AimbotKey = aimbot["AimbotKey"];
			if (aimbot.contains("AutoShoot")) Vars::Aimbot::AutoShoot = aimbot["AutoShoot"];
			if (aimbot.contains("DrawFOV")) Vars::Aimbot::DrawFOV = aimbot["DrawFOV"];
			if (aimbot.contains("FFAMode")) Vars::Aimbot::FFAMode = aimbot["FFAMode"];
			if (aimbot.contains("IgnoreFriends")) Vars::Aimbot::IgnoreFriends = aimbot["IgnoreFriends"];
		}
		
		// ESP settings
		if (config.contains("ESP"))
		{
			auto& esp = config["ESP"];
			if (esp.contains("Enabled")) Vars::ESP::Enabled = esp["Enabled"];
			if (esp.contains("Players")) Vars::ESP::Players = esp["Players"];
			if (esp.contains("PlayerBoxes")) Vars::ESP::PlayerBoxes = esp["PlayerBoxes"];
			if (esp.contains("PlayerNames")) Vars::ESP::PlayerNames = esp["PlayerNames"];
			if (esp.contains("PlayerHealth")) Vars::ESP::PlayerHealth = esp["PlayerHealth"];
			if (esp.contains("PlayerHealthBar")) Vars::ESP::PlayerHealthBar = esp["PlayerHealthBar"];
			if (esp.contains("PlayerWeapons")) Vars::ESP::PlayerWeapons = esp["PlayerWeapons"];
			if (esp.contains("PlayerConditions")) Vars::ESP::PlayerConditions = esp["PlayerConditions"];
			if (esp.contains("Items")) Vars::ESP::Items = esp["Items"];
			if (esp.contains("Ammo")) Vars::ESP::Ammo = esp["Ammo"];
			if (esp.contains("Health")) Vars::ESP::Health = esp["Health"];
			if (esp.contains("Weapons")) Vars::ESP::Weapons = esp["Weapons"];
			if (esp.contains("Powerups")) Vars::ESP::Powerups = esp["Powerups"];
		}
		
		// Misc settings
		if (config.contains("Misc"))
		{
			auto& misc = config["Misc"];
			if (misc.contains("Thirdperson")) Vars::Misc::Thirdperson = misc["Thirdperson"];
			if (misc.contains("ThirdpersonRight")) Vars::Misc::ThirdpersonRight = misc["ThirdpersonRight"];
			if (misc.contains("ThirdpersonUp")) Vars::Misc::ThirdpersonUp = misc["ThirdpersonUp"];
			if (misc.contains("ThirdpersonBack")) Vars::Misc::ThirdpersonBack = misc["ThirdpersonBack"];
		}
		
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 0, 255, 255, 255 }, "[Config] Loaded from: %s\n", path.c_str());
	}
	catch (const std::exception& e)
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[Config] Parse error: %s\n", e.what());
	}
}
