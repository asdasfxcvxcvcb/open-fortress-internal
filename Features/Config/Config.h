#pragma once

#include <string>

class CConfig
{
public:
	void SaveConfig(const std::string& configName = "config");
	void LoadConfig(const std::string& configName = "config");
	void SavePlayerList();
	void LoadPlayerList();
	
private:
	std::string GetConfigPath(const std::string& configName);
	std::string GetPlayerListPath();
	void CreateDirectoryIfNotExists(const std::string& path);
};

inline CConfig g_Config;
