#include "Config.h"
#include "../Vars.h"
#include "../../SDK/SDK.h"
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <ShlObj.h>

std::string CConfig::GetConfigPath(const std::string& configName)
{
	return "C:\\necromancer_OF2\\configs\\" + configName + ".json";
}

void CConfig::CreateDirectoryIfNotExists(const std::string& path)
{
	// Extract directory from full path
	size_t lastSlash = path.find_last_of("\\/");
	if (lastSlash != std::string::npos)
	{
		std::string dir = path.substr(0, lastSlash);
		
		// Create directory recursively
		std::string currentPath;
		std::istringstream ss(dir);
		std::string token;
		
		while (std::getline(ss, token, '\\'))
		{
			if (token.empty() || token.back() == ':')
			{
				currentPath += token + "\\";
				continue;
			}
			
			currentPath += token + "\\";
			CreateDirectoryA(currentPath.c_str(), NULL);
		}
	}
}

void CConfig::SaveConfig(const std::string& configName)
{
	std::string path = GetConfigPath(configName);
	CreateDirectoryIfNotExists(path);
	
	std::ofstream file(path);
	if (!file.is_open())
	{
		if (I::Cvar)
			I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "[Config] Failed to open file for writing: %s\n", path.c_str());
		return;
	}
	
	// Manual JSON writing (simple approach without external library)
	file << "{\n";
	
	// Aimbot settings
	file << "  \"Aimbot\": {\n";
	file << "    \"Enabled\": " << (Vars::Aimbot::Enabled ? "true" : "false") << ",\n";
	file << "    \"Mode\": " << Vars::Aimbot::Mode << ",\n";
	file << "    \"SmoothAmount\": " << Vars::Aimbot::SmoothAmount << ",\n";
	file << "    \"FOV\": " << Vars::Aimbot::FOV << ",\n";
	file << "    \"TargetSelection\": " << Vars::Aimbot::TargetSelection << ",\n";
	file << "    \"Hitbox\": " << Vars::Aimbot::Hitbox << ",\n";
	file << "    \"IgnoreCloaked\": " << (Vars::Aimbot::IgnoreCloaked ? "true" : "false") << ",\n";
	file << "    \"IgnoreInvulnerable\": " << (Vars::Aimbot::IgnoreInvulnerable ? "true" : "false") << ",\n";
	file << "    \"AimbotKey\": " << Vars::Aimbot::AimbotKey << ",\n";
	file << "    \"AutoShoot\": " << (Vars::Aimbot::AutoShoot ? "true" : "false") << ",\n";
	file << "    \"DrawFOV\": " << (Vars::Aimbot::DrawFOV ? "true" : "false") << ",\n";
	file << "    \"FFAMode\": " << (Vars::Aimbot::FFAMode ? "true" : "false") << "\n";
	file << "  },\n";
	
	// ESP settings
	file << "  \"ESP\": {\n";
	file << "    \"Enabled\": " << (Vars::ESP::Enabled ? "true" : "false") << ",\n";
	file << "    \"Players\": " << (Vars::ESP::Players ? "true" : "false") << ",\n";
	file << "    \"PlayerBoxes\": " << (Vars::ESP::PlayerBoxes ? "true" : "false") << ",\n";
	file << "    \"PlayerNames\": " << (Vars::ESP::PlayerNames ? "true" : "false") << ",\n";
	file << "    \"PlayerHealth\": " << (Vars::ESP::PlayerHealth ? "true" : "false") << ",\n";
	file << "    \"PlayerHealthBar\": " << (Vars::ESP::PlayerHealthBar ? "true" : "false") << ",\n";
	file << "    \"PlayerWeapons\": " << (Vars::ESP::PlayerWeapons ? "true" : "false") << ",\n";
	file << "    \"PlayerConditions\": " << (Vars::ESP::PlayerConditions ? "true" : "false") << ",\n";
	file << "    \"Items\": " << (Vars::ESP::Items ? "true" : "false") << ",\n";
	file << "    \"Ammo\": " << (Vars::ESP::Ammo ? "true" : "false") << ",\n";
	file << "    \"Health\": " << (Vars::ESP::Health ? "true" : "false") << ",\n";
	file << "    \"Weapons\": " << (Vars::ESP::Weapons ? "true" : "false") << ",\n";
	file << "    \"Powerups\": " << (Vars::ESP::Powerups ? "true" : "false") << "\n";
	file << "  }\n";
	
	file << "}\n";
	
	file.close();
	
	if (I::Cvar)
		I::Cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[Config] Saved successfully to: %s\n", path.c_str());
}

void CConfig::LoadConfig(const std::string& configName)
{
	std::string path = GetConfigPath(configName);
	std::ifstream file(path);
	
	if (!file.is_open())
		return; // Silently skip if no config exists
	
	std::string line;
	std::string currentSection;
	
	while (std::getline(file, line))
	{
		// Remove whitespace
		line.erase(0, line.find_first_not_of(" \t\r\n"));
		line.erase(line.find_last_not_of(" \t\r\n,") + 1);
		
		// Check for section
		if (line.find("\"Aimbot\"") != std::string::npos)
			currentSection = "Aimbot";
		else if (line.find("\"ESP\"") != std::string::npos)
			currentSection = "ESP";
		
		// Parse key-value pairs
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);
			
			// Clean up key and value
			key.erase(0, key.find_first_not_of(" \t\""));
			key.erase(key.find_last_not_of(" \t\"") + 1);
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t,") + 1);
			
			// Aimbot settings
			if (currentSection == "Aimbot")
			{
				if (key == "Enabled") Vars::Aimbot::Enabled = (value == "true");
				else if (key == "Mode") Vars::Aimbot::Mode = std::stoi(value);
				else if (key == "SmoothAmount") Vars::Aimbot::SmoothAmount = std::stof(value);
				else if (key == "FOV") Vars::Aimbot::FOV = std::stof(value);
				else if (key == "TargetSelection") Vars::Aimbot::TargetSelection = std::stoi(value);
				else if (key == "Hitbox") Vars::Aimbot::Hitbox = std::stoi(value);
				else if (key == "IgnoreCloaked") Vars::Aimbot::IgnoreCloaked = (value == "true");
				else if (key == "IgnoreInvulnerable") Vars::Aimbot::IgnoreInvulnerable = (value == "true");
				else if (key == "AimbotKey") Vars::Aimbot::AimbotKey = std::stoi(value);
				else if (key == "AutoShoot") Vars::Aimbot::AutoShoot = (value == "true");
				else if (key == "DrawFOV") Vars::Aimbot::DrawFOV = (value == "true");
				else if (key == "FFAMode") Vars::Aimbot::FFAMode = (value == "true");
			}
			// ESP settings
			else if (currentSection == "ESP")
			{
				if (key == "Enabled") Vars::ESP::Enabled = (value == "true");
				else if (key == "Players") Vars::ESP::Players = (value == "true");
				else if (key == "PlayerBoxes") Vars::ESP::PlayerBoxes = (value == "true");
				else if (key == "PlayerNames") Vars::ESP::PlayerNames = (value == "true");
				else if (key == "PlayerHealth") Vars::ESP::PlayerHealth = (value == "true");
				else if (key == "PlayerHealthBar") Vars::ESP::PlayerHealthBar = (value == "true");
				else if (key == "PlayerWeapons") Vars::ESP::PlayerWeapons = (value == "true");
				else if (key == "PlayerConditions") Vars::ESP::PlayerConditions = (value == "true");
				else if (key == "Items") Vars::ESP::Items = (value == "true");
				else if (key == "Ammo") Vars::ESP::Ammo = (value == "true");
				else if (key == "Health") Vars::ESP::Health = (value == "true");
				else if (key == "Weapons") Vars::ESP::Weapons = (value == "true");
				else if (key == "Powerups") Vars::ESP::Powerups = (value == "true");
			}
		}
	}
	
	file.close();
	
	if (I::Cvar)
		I::Cvar->ConsoleColorPrintf({ 0, 255, 255, 255 }, "[Config] Loaded successfully from: %s\n", path.c_str());
}
