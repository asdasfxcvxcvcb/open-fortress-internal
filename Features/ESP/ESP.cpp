#include "ESP.h"
#include "../Vars.h"
#include "../../vendor/imgui/imgui.h"

// Helper function to convert world position to screen using Source engine
static bool WorldToScreen(const Vector& vWorld, ImVec2& vScreen)
{
	Vector2D screenPos;
	if (!H::Draw.WorldPosToScreenPos(vWorld, screenPos))
		return false;
	
	vScreen.x = screenPos.x;
	vScreen.y = screenPos.y;
	return true;
}

// Helper to draw text with ImGui
static void DrawText(ImDrawList* drawList, const ImVec2& pos, ImU32 color, const char* text, bool centerX = false, bool centerY = false)
{
	if (!drawList || !text)
		return;

	ImVec2 textSize = ImGui::CalcTextSize(text);
	ImVec2 drawPos = pos;
	
	if (centerX)
		drawPos.x -= textSize.x / 2.0f;
	if (centerY)
		drawPos.y -= textSize.y / 2.0f;

	// Draw text outline (black)
	drawList->AddText(ImVec2(drawPos.x - 1, drawPos.y), IM_COL32(0, 0, 0, 255), text);
	drawList->AddText(ImVec2(drawPos.x + 1, drawPos.y), IM_COL32(0, 0, 0, 255), text);
	drawList->AddText(ImVec2(drawPos.x, drawPos.y - 1), IM_COL32(0, 0, 0, 255), text);
	drawList->AddText(ImVec2(drawPos.x, drawPos.y + 1), IM_COL32(0, 0, 0, 255), text);
	
	// Draw text
	drawList->AddText(drawPos, color, text);
}

void CFeatures_ESP::Render()
{
	if (I::EngineVGui->IsGameUIVisible() || !I::EngineClient->IsInGame())
		return;

	if (!Vars::ESP::Enabled)
		return;

	// Get ImGui draw list
	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	if (!drawList)
		return;

	int n, x, y, w, h;

	const int nMaxClients  = (I::EngineClient->GetMaxClients() + 1);
	const int nMaxEntities = I::ClientEntityList->GetMaxEntities();
	const int nLocalIndex  = I::EngineClient->GetLocalPlayer();

	C_TFPlayer* pLocal = I::ClientEntityList->GetClientEntity(nLocalIndex)->As<C_TFPlayer*>();

	if (!pLocal)
		return;

	ImVec2 screenPos;
	
	// Render world items (ammo, health, powerups, weapons)
	if (Vars::ESP::Items)
	{
		for (n = nMaxClients; n < nMaxEntities; n++)
		{
			C_BaseEntity* pEntity = C_BaseEntity::Instance(n);

			if (!pEntity || pEntity->IsDormant())
				continue;

			ClientClass* pCC = pEntity->GetClientClass();

			if (!pCC)
				continue;

			switch (pCC->m_ClassID)
			{
				case CTFAmmoPack:
				{
					if (!Vars::ESP::Ammo)
						break;
						
					if (!WorldToScreen(pEntity->WorldSpaceCenter(), screenPos))
						break;

					DrawText(drawList, screenPos, IM_COL32(150, 150, 150, 255), "ammo", true, true);
					break;
				}
				case CCondPowerup:
				{
					if (!Vars::ESP::Powerups)
						break;
						
					C_CondPowerup* pPowerup = pEntity->As<C_CondPowerup*>();

					if (!pPowerup || !WorldToScreen(pPowerup->WorldSpaceCenter(), screenPos))
						break;

					const int nModelIndex = pPowerup->m_nModelIndex();

					if (m_mapPowerups.find(nModelIndex) == m_mapPowerups.end())
						break;

					// Convert wchar_t to char
					char powerupName[64];
					size_t convertedChars = 0;
					wcstombs_s(&convertedChars, powerupName, sizeof(powerupName), m_mapPowerups[nModelIndex], _TRUNCATE);
					
					DrawText(drawList, screenPos, IM_COL32(255, 255, 0, 255), powerupName, true, true);
					screenPos.y += 15;

					if (pPowerup->m_bRespawning())
					{
						const float flRespawn = (pPowerup->m_flRespawnTick() - I::GlobalVarsBase->curtime);
						char respawnText[32];
						sprintf_s(respawnText, "%.1f", flRespawn);
						DrawText(drawList, screenPos, IM_COL32(255, 255, 0, 255), respawnText, true, true);
					}

					break;
				}
				case CWeaponSpawner:
				{
					if (!Vars::ESP::Weapons)
						break;
						
					C_WeaponSpawner* pSpawn = pEntity->As<C_WeaponSpawner*>();

					if (!pSpawn || !WorldToScreen(pSpawn->WorldSpaceCenter(), screenPos))
						break;

					const ImU32 color = pSpawn->m_bSuperWeapon() ? IM_COL32(255, 255, 0, 255) : IM_COL32(150, 150, 150, 255);

					char szArray[64];
					pSpawn->GetWeaponName(szArray);

					std::string szWeapon = (szArray + 10);
					std::transform(szWeapon.begin(), szWeapon.end(), szWeapon.begin(),
						[](unsigned char c) { return ::tolower(c); });

					DrawText(drawList, screenPos, color, szWeapon.c_str(), true, true);
					screenPos.y += 15;

					if (pSpawn->m_bRespawning())
					{
						const float flRespawn = (pSpawn->m_flRespawnTick() - I::GlobalVarsBase->curtime);
						char respawnText[32];
						sprintf_s(respawnText, "%.1fs", flRespawn);
						DrawText(drawList, screenPos, color, respawnText, true, true);
					}

					break;
				}
				case CBaseAnimating:
				{
					const int nModelIndex = pEntity->m_nModelIndex();

					if (Vars::ESP::Ammo && IsAmmo(nModelIndex) && WorldToScreen(pEntity->WorldSpaceCenter(), screenPos))
					{
						DrawText(drawList, screenPos, IM_COL32(150, 150, 150, 255), "ammo", true, true);
						break;
					}

					if (Vars::ESP::Health && IsHealth(nModelIndex) && WorldToScreen(pEntity->WorldSpaceCenter(), screenPos))
					{
						DrawText(drawList, screenPos, IM_COL32(0, 255, 0, 255), "health", true, true);
						break;
					}

					break;
				}
				default:
					break;
			}
		}
	}

	// Render players
	if (Vars::ESP::Players)
	{
		player_info_t pi;
		for (n = 1; n < nMaxClients; n++)
		{
			C_BaseEntity* pEntity = C_BaseEntity::Instance(n);

			if (!pEntity || pEntity->IsDormant())
				continue;

			C_TFPlayer* pPlayer = pEntity->As<C_TFPlayer*>();

			if (!pPlayer || pPlayer->deadflag())
				continue;

			const bool bIsLocal = (n == nLocalIndex);
			
			// Don't show local player ESP in first person (check both thirdperson setting and camera mode)
			if (bIsLocal && !Vars::Misc::Thirdperson && !I::Input->CAM_IsThirdPerson())
				continue;
			
			// Check ignore local setting
			if (bIsLocal && Vars::ESP::IgnoreLocal)
				continue;

			// Team filtering (skip for local player)
			if (!bIsLocal)
			{
				const bool bIsTeammate = pPlayer->InLocalTeam();
				if (Vars::ESP::IgnoreTeammates && bIsTeammate)
					continue;
				if (Vars::ESP::IgnoreEnemies && !bIsTeammate)
					continue;
			}

			// Friend filtering
			if (Vars::ESP::IgnoreFriends && I::EngineClient->GetPlayerInfo(n, &pi))
			{
				if (Vars::CustomFriends.find(pi.friendsID) != Vars::CustomFriends.end())
					continue;
			}

			// Cloaked filtering
			if (Vars::ESP::IgnoreCloaked && pPlayer->m_flInvisibility() >= 1.0f)
				continue;

			const int nHealth = pPlayer->GetHealth();
			const int nMaxHealth = pPlayer->GetMaxHealth();

			if (!nHealth || !nMaxHealth || !GetDynamicBounds(pPlayer, x, y, w, h))
				continue;

			float r, g, b;
			pPlayer->GetGlowEffectColor(&r, &g, &b);

			const ImU32 boxColor = IM_COL32(static_cast<int>(r * 255.0f), static_cast<int>(g * 255.0f), static_cast<int>(b * 255.0f), 255);
			
			// Get health color
			ImU32 healthColor;
			if (nHealth > nMaxHealth)
				healthColor = IM_COL32(0, 128, 255, 255); // Blue for overheal
			else if (nHealth < 25)
				healthColor = IM_COL32(255, 0, 0, 255); // Red
			else if (nHealth < 50)
				healthColor = IM_COL32(255, 128, 0, 255); // Orange
			else if (nHealth < 80)
				healthColor = IM_COL32(255, 255, 0, 255); // Yellow
			else
				healthColor = IM_COL32(0, 255, 0, 255); // Green

			// Draw box
			if (Vars::ESP::PlayerBoxes)
			{
				drawList->AddRect(ImVec2((float)x, (float)y), ImVec2((float)(x + w), (float)(y + h)), boxColor, 0.0f, 0, 2.0f);
				
				// Draw box outline
				drawList->AddRect(ImVec2((float)(x - 1), (float)(y - 1)), ImVec2((float)(x + w + 1), (float)(y + h + 1)), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
				drawList->AddRect(ImVec2((float)(x + 1), (float)(y + 1)), ImVec2((float)(x + w - 1), (float)(y + h - 1)), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
			}

			// Draw player name
			if (Vars::ESP::PlayerNames && I::EngineClient->GetPlayerInfo(n, &pi))
			{
				DrawText(drawList, ImVec2((float)(x + w / 2), (float)(y - 17)), IM_COL32(255, 255, 255, 255), pi.name, true, false);
			}

			const int nDrawX = x + w + 3;
			int nDrawY = y;

			// Draw health text
			if (Vars::ESP::PlayerHealth)
			{
				char healthText[32];
				sprintf_s(healthText, "%i", nHealth); // Show current health only
				DrawText(drawList, ImVec2((float)nDrawX, (float)nDrawY), healthColor, healthText);
				nDrawY += 15;
			}

			// Draw conditions
			if (Vars::ESP::PlayerConditions)
			{
				if (pPlayer->InCondUber())
				{
					DrawText(drawList, ImVec2((float)nDrawX, (float)nDrawY), IM_COL32(255, 255, 0, 255), "INVULNERABLE");
					nDrawY += 15;
				}

				if (pPlayer->InCondCrit())
				{
					DrawText(drawList, ImVec2((float)nDrawX, (float)nDrawY), IM_COL32(255, 255, 0, 255), "CRITS");
					nDrawY += 15;
				}

				if (pPlayer->InCondBerserk())
				{
					DrawText(drawList, ImVec2((float)nDrawX, (float)nDrawY), IM_COL32(255, 255, 0, 255), "BERSERK");
					nDrawY += 15;
				}

				if (pPlayer->InCondHaste())
				{
					DrawText(drawList, ImVec2((float)nDrawX, (float)nDrawY), IM_COL32(255, 255, 0, 255), "HASTE");
					nDrawY += 15;
				}

				if (pPlayer->InCondShield())
				{
					DrawText(drawList, ImVec2((float)nDrawX, (float)nDrawY), IM_COL32(255, 255, 0, 255), "SHIELD");
					nDrawY += 15;
				}
			}

			// Draw active weapon
			if (Vars::ESP::PlayerWeapons)
			{
				C_BaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
				if (pWeapon)
				{
					std::string szWeapon = (pWeapon->GetName() + 10);
					std::transform(szWeapon.begin(), szWeapon.end(), szWeapon.begin(),
						[](unsigned char c) { return std::tolower(c); });

					DrawText(drawList, ImVec2((float)(x + w / 2), (float)(y + h + 2)), IM_COL32(150, 150, 150, 255), szWeapon.c_str(), true, false);
				}
			}

			// Draw health bar
			if (Vars::ESP::PlayerHealthBar)
			{
				const float flMaxHealth = static_cast<float>(nMaxHealth);
				const float flHealth = U::Math.Clamp<float>(static_cast<float>(nHealth), 1.0f, flMaxHealth);

				static const int nWidth = 2;
				const int nHeight = h + 1;

				const float ratio = (flHealth / flMaxHealth);
				const int barHeight = static_cast<int>(nHeight * ratio);
				const int barY = y + nHeight - barHeight;
				
				// Draw health bar background
				drawList->AddRectFilled(ImVec2((float)(x - 5), (float)y), ImVec2((float)(x - 3), (float)(y + nHeight)), IM_COL32(0, 0, 0, 180));
				
				// Draw health bar
				drawList->AddRectFilled(ImVec2((float)(x - 5), (float)barY), ImVec2((float)(x - 3), (float)(y + nHeight)), healthColor);
				
				// Draw health bar outline
				drawList->AddRect(ImVec2((float)(x - 6), (float)(y - 1)), ImVec2((float)(x - 2), (float)(y + nHeight + 1)), IM_COL32(0, 0, 0, 255));
			}
		}
	}
}

void CFeatures_ESP::LevelInitPostEntity()
{
	// Clear box cache on map change
	m_mBoxCache.clear();
	
	m_vecAmmo.clear();
	{
		m_vecAmmo.push_back(I::ModelInfoClient->GetModelIndex("models/items/ammopack_large.mdl"));
		m_vecAmmo.push_back(I::ModelInfoClient->GetModelIndex("models/items/ammopack_medium.mdl"));
		m_vecAmmo.push_back(I::ModelInfoClient->GetModelIndex("models/items/ammopack_small.mdl"));
	}

	m_vecHealth.clear();
	{
		m_vecHealth.push_back(I::ModelInfoClient->GetModelIndex("models/items/medkit_large.mdl"));
		m_vecHealth.push_back(I::ModelInfoClient->GetModelIndex("models/items/medkit_medium.mdl"));
		m_vecHealth.push_back(I::ModelInfoClient->GetModelIndex("models/items/medkit_small.mdl"));
		m_vecHealth.push_back(I::ModelInfoClient->GetModelIndex("models/items/medkit_overheal.mdl"));
	}

	m_mapPowerups.clear();
	{
		m_mapPowerups.emplace(I::ModelInfoClient->GetModelIndex("models/pickups/pickup_powerup_crit.mdl"), L"POWERUP_CRIT");
		m_mapPowerups.emplace(I::ModelInfoClient->GetModelIndex("models/pickups/pickup_powerup_invis.mdl"), L"POWERUP_INVIS");
		m_mapPowerups.emplace(I::ModelInfoClient->GetModelIndex("models/pickups/pickup_powerup_defense.mdl"), L"POWERUP_SHIELD");
		m_mapPowerups.emplace(I::ModelInfoClient->GetModelIndex("models/pickups/pickup_powerup_knockout.mdl"), L"POWERUP_BERSERK");
		m_mapPowerups.emplace(I::ModelInfoClient->GetModelIndex("models/pickups/pickup_powerup_haste.mdl"), L"POWERUP_HASTE");
		m_mapPowerups.emplace(I::ModelInfoClient->GetModelIndex("models/pickups/pickup_powerup_megahealth.mdl"), L"POWERUP_MEGAHEALTH");
	}
}

bool CFeatures_ESP::IsAmmo(const int nModelIndex)
{
	size_t n; const size_t nMax = m_vecAmmo.size();
	for (n = 0; n < nMax; n++)
	{
		if (m_vecAmmo[n] == nModelIndex)
			return true;
	}

	return false;
}

bool CFeatures_ESP::IsHealth(const int nModelIndex)
{
	size_t n; const size_t nMax = m_vecHealth.size();
	for (n = 0; n < nMax; n++)
	{
		if (m_vecHealth[n] == nModelIndex)
			return true;
	}

	return false;
}

bool CFeatures_ESP::GetDynamicBounds(C_BaseEntity* pEntity, int& x, int& y, int& w, int& h)
{
	Vector vPoints[8];
	
	// Use the entity's transform directly - it should already be interpolated
	const matrix3x4_t& transform = pEntity->RenderableToWorldTransform();
	
	U::Math.BuildTransformedBox(vPoints, pEntity->m_vecMins(), pEntity->m_vecMaxs(), transform);

	Vector2D flb, brt, blb, frt, frb, brb, blt, flt;

	if (H::Draw.WorldPosToScreenPos(vPoints[3], flb) && H::Draw.WorldPosToScreenPos(vPoints[5], brt)
		&& H::Draw.WorldPosToScreenPos(vPoints[0], blb) && H::Draw.WorldPosToScreenPos(vPoints[4], frt)
		&& H::Draw.WorldPosToScreenPos(vPoints[2], frb) && H::Draw.WorldPosToScreenPos(vPoints[1], brb)
		&& H::Draw.WorldPosToScreenPos(vPoints[6], blt) && H::Draw.WorldPosToScreenPos(vPoints[7], flt)
		&& H::Draw.WorldPosToScreenPos(vPoints[6], blt) && H::Draw.WorldPosToScreenPos(vPoints[7], flt))
	{
		const Vector2D arr[8] = { flb, brt, blb, frt, frb, brb, blt, flt };

		float left = flb.x;
		float top = flb.y;
		float righ = flb.x;
		float bottom = flb.y;

		for (int n = 1; n < 8; n++)
		{
			if (left > arr[n].x)
				left = arr[n].x;

			if (top < arr[n].y)
				top = arr[n].y;

			if (righ < arr[n].x)
				righ = arr[n].x;

			if (bottom > arr[n].y)
				bottom = arr[n].y;
		}

		// Calculate raw box values
		float x_ = left;
		float y_ = bottom;
		float w_ = (righ - left);
		float h_ = (top - bottom);
		
		// Slightly narrow the box for players (looks better)
		x_ += (w_ / 8.0f);
		w_ -= ((w_ / 8.0f) * 2.0f);
		
		// Apply smoothing to reduce jitter
		const int entIndex = pEntity->entindex();
		const float currentTime = I::GlobalVarsBase->curtime;
		const float smoothFactor = 0.5f; // Higher = more smoothing but more lag
		
		if (m_mBoxCache.find(entIndex) != m_mBoxCache.end())
		{
			auto& cache = m_mBoxCache[entIndex];
			
			// Only smooth if update is recent (within 0.5 seconds)
			if (currentTime - cache.lastUpdateTime < 0.5f)
			{
				// Lerp towards new position
				x_ = cache.x + (x_ - cache.x) * smoothFactor;
				y_ = cache.y + (y_ - cache.y) * smoothFactor;
				w_ = cache.w + (w_ - cache.w) * smoothFactor;
				h_ = cache.h + (h_ - cache.h) * smoothFactor;
			}
		}
		
		// Update cache
		m_mBoxCache[entIndex] = { x_, y_, w_, h_, currentTime };

		x = static_cast<int>(x_);
		y = static_cast<int>(y_);
		w = static_cast<int>(w_);
		h = static_cast<int>(h_);

		return !(x > H::Draw.m_nScreenW || (x + w) < 0 || y > H::Draw.m_nScreenH || (y + h) < 0);
	}

	return false;
}