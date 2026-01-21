#pragma once

#include <vector>
#include <string>
#include <chrono>

class CChat
{
private:
	std::vector<std::string> m_vSpamMessages;
	int m_nCurrentMessageIndex = 0;
	std::string m_sFilePath = "C:\\necromancer_OF2\\chat\\chatspam.txt";
	std::chrono::steady_clock::time_point m_LastSpamTime;

public:
	void Initialize();
	void RefreshMessages();
	void Run(); // Call this every frame
	std::string GetNextMessage();
	bool HasMessages() const { return !m_vSpamMessages.empty(); }
	int GetMessageCount() const { return static_cast<int>(m_vSpamMessages.size()); }
	const std::string& GetFilePath() const { return m_sFilePath; }
};

namespace F { inline CChat Chat; }
