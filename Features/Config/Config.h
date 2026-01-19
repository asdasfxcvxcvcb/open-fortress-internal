#pragma once

#include <string>

class CConfig
{
public:
	void SaveConfig(const std::string& configName = "config");
	void LoadConfig(const std::string& configName = "config");
	
private:
	std::string GetConfigPath(const std::string& configName);
	void CreateDirectoryIfNotExists(const std::string& path);
};

inline CConfig g_Config;
