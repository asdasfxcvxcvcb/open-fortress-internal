#include "Chat.h"
#include "../Vars.h"
#include "../../SDK/SDK.h"
#include <fstream>
#include <Windows.h>
#include <sstream>
#include <chrono>

void CChat::Initialize()
{
	// Create directory if it doesn't exist
	CreateDirectoryA("C:\\necromancer_OF2", NULL);
	CreateDirectoryA("C:\\necromancer_OF2\\chat", NULL);

	// Check if file exists
	std::ifstream checkFile(m_sFilePath);
	bool fileExists = checkFile.good();
	checkFile.close();

	// Only create default file if it doesn't exist
	if (!fileExists)
	{
		std::ofstream file(m_sFilePath);
		if (file.is_open())
		{
			file << "i won your loser\n";
			file << "necromancer is the best cheat\n";
			file << "noobs\n";
			file.close();
		}
	}

	// Load messages
	RefreshMessages();
	
	// Initialize timer
	m_LastSpamTime = std::chrono::steady_clock::now();
}

void CChat::RefreshMessages()
{
	m_vSpamMessages.clear();
	m_nCurrentMessageIndex = 0;

	std::ifstream file(m_sFilePath);
	if (!file.is_open())
		return;

	std::string line;
	while (std::getline(file, line))
	{
		// Trim whitespace and skip empty lines
		line.erase(0, line.find_first_not_of(" \t\r\n"));
		line.erase(line.find_last_not_of(" \t\r\n") + 1);
		
		if (!line.empty())
		{
			m_vSpamMessages.push_back(line);
		}
	}

	file.close();
}

std::string CChat::GetNextMessage()
{
	if (m_vSpamMessages.empty())
		return "";

	std::string message = m_vSpamMessages[m_nCurrentMessageIndex];
	
	// Move to next message (cycle through)
	m_nCurrentMessageIndex = (m_nCurrentMessageIndex + 1) % m_vSpamMessages.size();
	
	return message;
}

void CChat::Run()
{
	// Check if chat spammer is enabled
	if (!Vars::Misc::ChatSpammer)
		return;
	
	// Check if we have messages
	if (m_vSpamMessages.empty())
		return;
	
	// Check if engine is available and connected
	if (!I::EngineClient || !I::EngineClient->IsConnected() || !I::EngineClient->IsInGame())
		return;
	
	// Check if enough time has passed
	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_LastSpamTime).count() / 1000.0f;
	
	if (elapsed >= Vars::Misc::ChatSpammerInterval)
	{
		// Get next message and send it
		std::string message = GetNextMessage();
		if (!message.empty())
		{
			// Send to chat using engine command
			std::string cmd = "say " + message;
			I::EngineClient->ClientCmd_Unrestricted(cmd.c_str());
			
			// Update last spam time
			m_LastSpamTime = now;
		}
	}
}
