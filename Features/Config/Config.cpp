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
	
	// Automatically save all registered variables
	for (const auto& var : Vars::RegisteredVars)
	{
		var.save(config);
	}
	
	// Save binds
	config["Binds"] = json::array();
	for (const auto& bind : Vars::Binds)
	{
		json bindJson;
		bindJson["name"] = bind.name;
		bindJson["key"] = bind.key;
		bindJson["isToggle"] = bind.isToggle;
		
		bindJson["boolSettings"] = json::object();
		for (const auto& [key, value] : bind.boolSettings)
			bindJson["boolSettings"][key] = value;
		
		bindJson["intSettings"] = json::object();
		for (const auto& [key, value] : bind.intSettings)
			bindJson["intSettings"][key] = value;
		
		bindJson["floatSettings"] = json::object();
		for (const auto& [key, value] : bind.floatSettings)
			bindJson["floatSettings"][key] = value;
		
		config["Binds"].push_back(bindJson);
	}
	
	// Save friends
	config["Friends"] = json::array();
	for (const auto& friendID : Vars::CustomFriends)
	{
		config["Friends"].push_back(friendID);
	}
	
	std::ofstream file(path);
	if (file.is_open())
	{
		file << config.dump(4);
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
		
		// Automatically load all registered variables
		for (const auto& var : Vars::RegisteredVars)
		{
			var.load(config);
		}
		
		// Load binds
		if (config.contains("Binds") && config["Binds"].is_array())
		{
			Vars::Binds.clear();
			for (const auto& bindJson : config["Binds"])
			{
				Vars::BindConfig bind;
				if (bindJson.contains("name")) bind.name = bindJson["name"];
				if (bindJson.contains("key")) bind.key = bindJson["key"];
				if (bindJson.contains("isToggle")) bind.isToggle = bindJson["isToggle"];
				
				bind.isActive = false;
				
				if (bindJson.contains("boolSettings") && bindJson["boolSettings"].is_object())
				{
					for (auto& [key, value] : bindJson["boolSettings"].items())
						bind.boolSettings[key] = value;
				}
				
				if (bindJson.contains("intSettings") && bindJson["intSettings"].is_object())
				{
					for (auto& [key, value] : bindJson["intSettings"].items())
						bind.intSettings[key] = value;
				}
				
				if (bindJson.contains("floatSettings") && bindJson["floatSettings"].is_object())
				{
					for (auto& [key, value] : bindJson["floatSettings"].items())
						bind.floatSettings[key] = value;
				}
				
				bind.previousBoolSettings.clear();
				bind.previousIntSettings.clear();
				bind.previousFloatSettings.clear();
				
				Vars::Binds.push_back(bind);
			}
		}
		
		// Load friends
		if (config.contains("Friends") && config["Friends"].is_array())
		{
			Vars::CustomFriends.clear();
			for (const auto& friendID : config["Friends"])
			{
				if (friendID.is_number_unsigned())
					Vars::CustomFriends.insert(friendID.get<uint32>());
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
	// Deprecated - friends are now saved in main config
	SaveConfig();
}

void CConfig::LoadPlayerList()
{
	// Deprecated - friends are now loaded from main config
	LoadConfig();
}
