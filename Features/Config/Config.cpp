#include "Config.h"
#include "../Vars.h"
#include "../Menu/Menu.h"
#include "../../SDK/SDK.h"
#include "../../vendor/nlohmann/json.hpp"
#include <fstream>
#include <Windows.h>

using json = nlohmann::json;

std::string CConfig::GetConfigPath(const std::string& configName)
{
	return "C:\\necromancer_OF2\\configs\\" + configName + ".json";
}

std::string CConfig::GetPlayerListPath()
{
	return "C:\\necromancer_OF2\\playerlist.json";
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
	
	// Menu Colors
	const float* menuBg = F::Menu.GetMenuBackgroundColor();
	const float* menuText = F::Menu.GetMenuTextColor();
	const float* menuAccent = F::Menu.GetMenuAccentColor();
	const float* windowColor = F::Menu.GetWindowColor();
	const float* fovColor = F::Menu.GetAimbotFOVColor();
	
	config["Colors"]["MenuBackground"] = json::array({menuBg[0], menuBg[1], menuBg[2], menuBg[3]});
	config["Colors"]["MenuText"] = json::array({menuText[0], menuText[1], menuText[2], menuText[3]});
	config["Colors"]["MenuAccent"] = json::array({menuAccent[0], menuAccent[1], menuAccent[2], menuAccent[3]});
	config["Colors"]["WindowColor"] = json::array({windowColor[0], windowColor[1], windowColor[2], windowColor[3]});
	config["Colors"]["AimbotFOV"] = json::array({fovColor[0], fovColor[1], fovColor[2], fovColor[3]});
	
	// Binds
	config["Binds"] = json::array();
	for (const auto& bind : Vars::Binds)
	{
		json bindJson;
		bindJson["name"] = bind.name;
		bindJson["key"] = bind.key;
		bindJson["isToggle"] = bind.isToggle;
		
		// Save bool settings (excluding colors)
		bindJson["boolSettings"] = json::object();
		for (const auto& [key, value] : bind.boolSettings)
			bindJson["boolSettings"][key] = value;
		
		// Save int settings
		bindJson["intSettings"] = json::object();
		for (const auto& [key, value] : bind.intSettings)
			bindJson["intSettings"][key] = value;
		
		// Save float settings
		bindJson["floatSettings"] = json::object();
		for (const auto& [key, value] : bind.floatSettings)
			bindJson["floatSettings"][key] = value;
		
		config["Binds"].push_back(bindJson);
	}
	
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
		
		// Menu Colors
		if (config.contains("Colors"))
		{
			auto& colors = config["Colors"];
			if (colors.contains("MenuBackground") && colors["MenuBackground"].is_array() && colors["MenuBackground"].size() == 4)
				F::Menu.SetMenuBackgroundColor(colors["MenuBackground"][0], colors["MenuBackground"][1], colors["MenuBackground"][2], colors["MenuBackground"][3]);
			if (colors.contains("MenuText") && colors["MenuText"].is_array() && colors["MenuText"].size() == 4)
				F::Menu.SetMenuTextColor(colors["MenuText"][0], colors["MenuText"][1], colors["MenuText"][2], colors["MenuText"][3]);
			if (colors.contains("MenuAccent") && colors["MenuAccent"].is_array() && colors["MenuAccent"][3] == 4)
				F::Menu.SetMenuAccentColor(colors["MenuAccent"][0], colors["MenuAccent"][1], colors["MenuAccent"][2], colors["MenuAccent"][3]);
			if (colors.contains("WindowColor") && colors["WindowColor"].is_array() && colors["WindowColor"].size() == 4)
				F::Menu.SetWindowColor(colors["WindowColor"][0], colors["WindowColor"][1], colors["WindowColor"][2], colors["WindowColor"][3]);
			if (colors.contains("AimbotFOV") && colors["AimbotFOV"].is_array() && colors["AimbotFOV"].size() == 4)
				F::Menu.SetAimbotFOVColor(colors["AimbotFOV"][0], colors["AimbotFOV"][1], colors["AimbotFOV"][2], colors["AimbotFOV"][3]);
		}
		
		// Binds
		if (config.contains("Binds") && config["Binds"].is_array())
		{
			Vars::Binds.clear();
			for (const auto& bindJson : config["Binds"])
			{
				Vars::BindConfig bind;
				if (bindJson.contains("name")) bind.name = bindJson["name"];
				if (bindJson.contains("key")) bind.key = bindJson["key"];
				if (bindJson.contains("isToggle")) bind.isToggle = bindJson["isToggle"];
				
				// Ensure bind starts inactive
				bind.isActive = false;
				
				// Load bool settings
				if (bindJson.contains("boolSettings") && bindJson["boolSettings"].is_object())
				{
					for (auto& [key, value] : bindJson["boolSettings"].items())
						bind.boolSettings[key] = value;
				}
				
				// Load int settings
				if (bindJson.contains("intSettings") && bindJson["intSettings"].is_object())
				{
					for (auto& [key, value] : bindJson["intSettings"].items())
						bind.intSettings[key] = value;
				}
				
				// Load float settings
				if (bindJson.contains("floatSettings") && bindJson["floatSettings"].is_object())
				{
					for (auto& [key, value] : bindJson["floatSettings"].items())
						bind.floatSettings[key] = value;
				}
				
				// Clear previous settings (they should be empty on load)
				bind.previousBoolSettings.clear();
				bind.previousIntSettings.clear();
				bind.previousFloatSettings.clear();
				
				Vars::Binds.push_back(bind);
			}
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

void CConfig::SavePlayerList()
{
	std::string path = GetPlayerListPath();
	CreateDirectoryIfNotExists(path);
	
	json playerList;
	playerList["friends"] = json::array();
	
	for (const auto& friendID : Vars::CustomFriends)
	{
		playerList["friends"].push_back(friendID);
	}
	
	std::ofstream file(path);
	if (file.is_open())
	{
		file << playerList.dump(4);
		file.close();
		
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[PlayerList] Saved to: %s\n", path.c_str());
	}
	else if (I::Cvar)
	{
		I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[PlayerList] Failed to save: %s\n", path.c_str());
	}
}

void CConfig::LoadPlayerList()
{
	std::string path = GetPlayerListPath();
	std::ifstream file(path);
	
	if (!file.is_open())
		return;
	
	try
	{
		json playerList;
		file >> playerList;
		file.close();
		
		Vars::CustomFriends.clear();
		
		if (playerList.contains("friends") && playerList["friends"].is_array())
		{
			for (const auto& friendID : playerList["friends"])
			{
				if (friendID.is_number_unsigned())
					Vars::CustomFriends.insert(friendID.get<uint32>());
			}
		}
		
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 0, 255, 255, 255 }, "[PlayerList] Loaded %d friends from: %s\n", Vars::CustomFriends.size(), path.c_str());
	}
	catch (const std::exception& e)
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[PlayerList] Parse error: %s\n", e.what());
	}
}
