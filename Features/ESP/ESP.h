#pragma once

#include "../../SDK/SDK.h"
#include <unordered_map>

class CFeatures_ESP
{
public:
	void Render();
	void LevelInitPostEntity();

private:
	bool IsAmmo(const int nModelIndex);
	bool IsHealth(const int nModelIndex);
	bool GetDynamicBounds(C_BaseEntity* pEntity, int& x, int& y, int& w, int& h);

private:
	std::vector<int> m_vecHealth = { };
	std::vector<int> m_vecAmmo = { };
	std::map<int, const wchar_t*> m_mapPowerups = { };
	
	// Smoothing cache for ESP boxes to reduce jitter
	struct BoxCache {
		float x, y, w, h;
		float lastUpdateTime;
	};
	std::unordered_map<int, BoxCache> m_mBoxCache;
};

namespace F { inline CFeatures_ESP ESP; }