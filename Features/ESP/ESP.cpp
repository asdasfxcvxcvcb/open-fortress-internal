#include "ESP.h"
#include "../Vars.h"
#include "../../vendor/imgui/imgui.h"
#include "../../Util/Math/Math.h"

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
		drawPos.x -= textSize.x * 0.5f;
	if (centerY)
		drawPos.y -= textSize.y * 0.5f;

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
	static bool s_bIgnoreTeam = false;

	if (!Vars::ESP::Enabled)
		return;

	if (I::EngineVGui->IsGameUIVisible() || !I::EngineClient->IsInGame())
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

					if (Vars::ESP::Ammo)
					{
						const auto& Model = pEntity->GetModel();
						if (!Model) continue;

						const char* szModelName = I::ModelInfoClient->GetModelName(Model);
						if (!szModelName) continue;

						switch (FNV1A::Hash(szModelName))
						{
						case FNV1A::HashConst("models/items/ammopack_medium.mdl"):
						case FNV1A::HashConst("models/items/ammopack_small.mdl"):
						case FNV1A::HashConst("models/items/ammopack_large.mdl"):
							if (WorldToScreen(pEntity->WorldSpaceCenter(), screenPos))
							{
								DrawText(drawList, screenPos, IM_COL32(150, 150, 150, 255), "ammo", true, true);
								break;
							}
							break;
						}
					}

					if (Vars::ESP::Health)
					{
						const auto& Model = pEntity->GetModel();
						if (!Model) continue;

						const char* szModelName = I::ModelInfoClient->GetModelName(Model);
						if (!szModelName) continue;

						switch (FNV1A::Hash(szModelName))
						{
						case FNV1A::HashConst("models/items/medkit_large.mdl"):
						case FNV1A::HashConst("models/items/medkit_medium.mdl"):
						case FNV1A::HashConst("models/items/medkit_small.mdl"):
						case FNV1A::HashConst("models/items/medkit_overheal.mdl"):
							if (WorldToScreen(pEntity->WorldSpaceCenter(), screenPos))
							{
								DrawText(drawList, screenPos, IM_COL32(0, 255, 0, 255), "health", true, true);
								break;
							}
							break;
						}
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
		if (n == nLocalIndex)
			continue;

		C_BaseEntity* pEntity = C_BaseEntity::Instance(n);

		if (!pEntity || pEntity->IsDormant())
			continue;

		C_TFPlayer* pPlayer = pEntity->As<C_TFPlayer*>();

		if (pPlayer->deadflag())
			continue;

		if (s_bIgnoreTeam && pPlayer->InLocalTeam())
			continue;

		const int nHealth = pPlayer->GetHealth();
		const int nMaxHealth = pPlayer->GetMaxHealth();

		if (!GetDynamicBounds(pPlayer, x, y, w, h))
			continue;

		float r, g, b;
		pPlayer->GetGlowEffectColor(&r, &g, &b);

		const ImU32 boxColor = IM_COL32(static_cast<int>(r * 255.0f), static_cast<int>(g * 255.0f), static_cast<int>(b * 255.0f), 255);
		
		// Get health color
		ImU32 healthColor = IM_COL32(0, 255, 0, 255);
		if (nHealth > nMaxHealth)
			healthColor = IM_COL32(0, 128, 255, 255); // Blue for overheal
		else if (nHealth < 25)
			healthColor = IM_COL32(255, 0, 0, 255); // Red
		else if (nHealth < 50)
			healthColor = IM_COL32(255, 128, 0, 255); // Orange
		else if (nHealth < 80)
			healthColor = IM_COL32(255, 255, 0, 255); // Yellow

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
			sprintf(healthText, "%i", nHealth); // Show current health only
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
			const float flHealth = std::clamp<float>(static_cast<float>(nHealth), 1.0f, flMaxHealth);

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

bool CFeatures_ESP::GetDynamicBounds(C_BaseEntity* pEntity, int& x, int& y, int& w, int& h)
{
	Vector vPoints[8];
	U::Math.BuildTransformedBox(vPoints, pEntity->m_vecMins(), pEntity->m_vecMaxs(), pEntity->RenderableToWorldTransform());

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

		x = static_cast<int>(left);
		y = static_cast<int>(bottom);
		w = static_cast<int>((righ - left));
		h = static_cast<int>((top - bottom));

		return !(x > H::Draw.m_nScreenW || (x + w) < 0 || y > H::Draw.m_nScreenH || (y + h) < 0);
	}

	return false;
}