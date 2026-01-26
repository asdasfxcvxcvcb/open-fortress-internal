#include "ESP.h"
#include "../Vars.h"
#include "../../vendor/imgui/imgui.h"
#include "../Backtrack/Backtrack.h"

// ============================================================================
// Unified ESP Drawing Constants
// ============================================================================
namespace ESPConstants
{
	// Line and shape thickness
	constexpr float LINE_THICKNESS = 1.0f;
	constexpr float BOX_THICKNESS = 2.0f;
	constexpr float CIRCLE_THICKNESS = 1.0f;
	
	// Box outline offsets
	constexpr int BOX_OUTLINE_OUTER = 1;  // Pixels outside main box
	constexpr int BOX_OUTLINE_INNER = 1;  // Pixels inside main box
	
	// Text outline
	constexpr int TEXT_OUTLINE_OFFSET = 1;
	
	// Colors
	const Color COLOR_OUTLINE(0, 0, 0, 255);
	const Color COLOR_HEALTHBAR_BG(0, 0, 0, 180);
	
	// Health bar
	constexpr int HEALTHBAR_WIDTH = 2;
	constexpr int HEALTHBAR_OFFSET = 5;  // Distance from box
	constexpr int HEALTHBAR_OUTLINE_THICKNESS = 1;
	
	// Text offsets
	constexpr int NAME_OFFSET_Y = 15;
	constexpr int INFO_OFFSET_X = 3;
	constexpr int INFO_LINE_HEIGHT = 12;
	constexpr int WEAPON_OFFSET_Y = 2;
	
	// Circle segments
	constexpr int CIRCLE_SEGMENTS = 16;
}

// ============================================================================
// Draw Interface Implementations
// ============================================================================

void CFeatures_ESP::ImGuiDraw::Line(int x1, int y1, int x2, int y2, const Color& color, float thickness)
{
	if (!m_pDrawList) return;
	m_pDrawList->AddLine(ImVec2((float)x1, (float)y1), ImVec2((float)x2, (float)y2), 
						 IM_COL32(color.r(), color.g(), color.b(), color.a()), ESPConstants::LINE_THICKNESS);
}

void CFeatures_ESP::ImGuiDraw::OutlinedRect(int x, int y, int w, int h, const Color& color)
{
	if (!m_pDrawList) return;
	// Draw main box first
	m_pDrawList->AddRect(ImVec2((float)x, (float)y), ImVec2((float)(x + w), (float)(y + h)), 
						 IM_COL32(color.r(), color.g(), color.b(), color.a()), 0.0f, 0, ESPConstants::BOX_THICKNESS);
	// Draw black outlines on top
	const auto& outline = ESPConstants::COLOR_OUTLINE;
	m_pDrawList->AddRect(ImVec2((float)(x - ESPConstants::BOX_OUTLINE_OUTER), (float)(y - ESPConstants::BOX_OUTLINE_OUTER)), 
						 ImVec2((float)(x + w + ESPConstants::BOX_OUTLINE_OUTER), (float)(y + h + ESPConstants::BOX_OUTLINE_OUTER)), 
						 IM_COL32(outline.r(), outline.g(), outline.b(), outline.a()), 0.0f, 0, ESPConstants::LINE_THICKNESS);
	m_pDrawList->AddRect(ImVec2((float)(x + ESPConstants::BOX_OUTLINE_INNER), (float)(y + ESPConstants::BOX_OUTLINE_INNER)), 
						 ImVec2((float)(x + w - ESPConstants::BOX_OUTLINE_INNER), (float)(y + h - ESPConstants::BOX_OUTLINE_INNER)), 
						 IM_COL32(outline.r(), outline.g(), outline.b(), outline.a()), 0.0f, 0, ESPConstants::LINE_THICKNESS);
}

void CFeatures_ESP::ImGuiDraw::FilledRect(int x, int y, int w, int h, const Color& color)
{
	if (!m_pDrawList) return;
	m_pDrawList->AddRectFilled(ImVec2((float)x, (float)y), ImVec2((float)(x + w), (float)(y + h)), 
							   IM_COL32(color.r(), color.g(), color.b(), color.a()));
}

void CFeatures_ESP::ImGuiDraw::Circle(int x, int y, float radius, const Color& color, float thickness)
{
	if (!m_pDrawList) return;
	m_pDrawList->AddCircle(ImVec2((float)x, (float)y), radius, 
						   IM_COL32(color.r(), color.g(), color.b(), color.a()), 
						   ESPConstants::CIRCLE_SEGMENTS, ESPConstants::CIRCLE_THICKNESS);
}

void CFeatures_ESP::ImGuiDraw::Text(int x, int y, const Color& color, int align, const char* text)
{
	if (!m_pDrawList || !text) return;
	ImVec2 textSize = ImGui::CalcTextSize(text);
	ImVec2 drawPos((float)x, (float)y);
	if (align & 0x8) drawPos.x -= textSize.x / 2.0f; // TXT_CENTERX
	
	// Draw text outline
	const auto& outline = ESPConstants::COLOR_OUTLINE;
	const int offset = ESPConstants::TEXT_OUTLINE_OFFSET;
	m_pDrawList->AddText(ImVec2(drawPos.x - offset, drawPos.y), IM_COL32(outline.r(), outline.g(), outline.b(), outline.a()), text);
	m_pDrawList->AddText(ImVec2(drawPos.x + offset, drawPos.y), IM_COL32(outline.r(), outline.g(), outline.b(), outline.a()), text);
	m_pDrawList->AddText(ImVec2(drawPos.x, drawPos.y - offset), IM_COL32(outline.r(), outline.g(), outline.b(), outline.a()), text);
	m_pDrawList->AddText(ImVec2(drawPos.x, drawPos.y + offset), IM_COL32(outline.r(), outline.g(), outline.b(), outline.a()), text);
	
	// Draw text
	m_pDrawList->AddText(drawPos, IM_COL32(color.r(), color.g(), color.b(), color.a()), text);
}

void CFeatures_ESP::SurfaceDraw::Line(int x1, int y1, int x2, int y2, const Color& color, float thickness)
{
	H::Draw.Line(x1, y1, x2, y2, color);
}

void CFeatures_ESP::SurfaceDraw::OutlinedRect(int x, int y, int w, int h, const Color& color)
{
	// Draw main box first (2 pixels thick by drawing twice with offset)
	const int thickness = static_cast<int>(ESPConstants::BOX_THICKNESS);
	for (int i = 0; i < thickness; i++)
	{
		H::Draw.Line(x, y + i, x + w, y + i, color); // Top
		H::Draw.Line(x, y + h - i, x + w, y + h - i, color); // Bottom
		H::Draw.Line(x + i, y, x + i, y + h, color); // Left
		H::Draw.Line(x + w - i, y, x + w - i, y + h, color); // Right
	}
	
	// Draw black outlines on top
	const auto& outline = ESPConstants::COLOR_OUTLINE;
	const int outerOffset = ESPConstants::BOX_OUTLINE_OUTER;
	const int innerOffset = ESPConstants::BOX_OUTLINE_INNER;
	
	// Outer outline
	H::Draw.Line(x - outerOffset, y - outerOffset, x + w + outerOffset, y - outerOffset, outline); // Top
	H::Draw.Line(x - outerOffset, y + h + outerOffset, x + w + outerOffset, y + h + outerOffset, outline); // Bottom
	H::Draw.Line(x - outerOffset, y - outerOffset, x - outerOffset, y + h + outerOffset, outline); // Left
	H::Draw.Line(x + w + outerOffset, y - outerOffset, x + w + outerOffset, y + h + outerOffset, outline); // Right
	
	// Inner outline
	H::Draw.Line(x + innerOffset, y + innerOffset, x + w - innerOffset, y + innerOffset, outline); // Top
	H::Draw.Line(x + innerOffset, y + h - innerOffset, x + w - innerOffset, y + h - innerOffset, outline); // Bottom
	H::Draw.Line(x + innerOffset, y + innerOffset, x + innerOffset, y + h - innerOffset, outline); // Left
	H::Draw.Line(x + w - innerOffset, y + innerOffset, x + w - innerOffset, y + h - innerOffset, outline); // Right
}

void CFeatures_ESP::SurfaceDraw::FilledRect(int x, int y, int w, int h, const Color& color)
{
	H::Draw.Rect(x, y, w, h, color);
}

void CFeatures_ESP::SurfaceDraw::Circle(int x, int y, float radius, const Color& color, float thickness)
{
	H::Draw.OutlinedCircle(x, y, static_cast<int>(radius), ESPConstants::CIRCLE_SEGMENTS, color);
}

void CFeatures_ESP::SurfaceDraw::Text(int x, int y, const Color& color, int align, const char* text)
{
	// Surface fonts already have FONTFLAG_OUTLINE, so don't add extra outline
	H::Draw.String(EFonts::ESP, x, y, color, static_cast<short>(align), text);
}

// ============================================================================
// Original Helper Functions
// ============================================================================

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

// ============================================================================
// Unified ESP Rendering - Add new features here once
// ============================================================================

void CFeatures_ESP::RenderPlayerSkeleton(IDrawInterface* draw, C_TFPlayer* pPlayer, const Color& customColor, matrix3x4_t* pBoneMatrix)
{
	if (!pPlayer) return;

	const auto pModel = pPlayer->GetModel();
	if (!pModel) return;

	const auto pHdr = I::ModelInfoClient->GetStudiomodel(pModel);
	if (!pHdr) return;

	matrix3x4_t BoneMatrix[128];
	matrix3x4_t* pBones = pBoneMatrix;

	if (!pBones)
	{
		if (!pPlayer->SetupBones(BoneMatrix, 128, 0x100, 0.0f)) return;
		pBones = BoneMatrix;
	}

	Color skeletonColor = customColor;
	Color outlineColor(0, 0, 0, 255);

	// Find head
	Vector vHeadPos, vNeckPos;
	bool bHasHeadPos = false;
	float headRadius = 0.0f;
	
	const auto pSet = pHdr->pHitboxSet(pPlayer->m_nHitboxSet());
	if (pSet)
	{
		const auto pHeadBox = pSet->pHitbox(0);
		if (pHeadBox)
		{
			Vector vMin, vMax;
			U::Math.VectorTransform(pHeadBox->bbmin, pBones[pHeadBox->bone], vMin);
			U::Math.VectorTransform(pHeadBox->bbmax, pBones[pHeadBox->bone], vMax);
			vHeadPos = (vMin + vMax) * 0.5f;
			
			if (pHeadBox->bone > 0)
			{
				mstudiobone_t* pHeadBone = pHdr->pBone(pHeadBox->bone);
				if (pHeadBone && pHeadBone->parent != -1)
				{
					U::Math.VectorTransform(Vector(0, 0, 0), pBones[pHeadBone->parent], vNeckPos);
					Vector2D screenHead, screenNeck;
					if (H::Draw.WorldPosToScreenPos(vHeadPos, screenHead) && H::Draw.WorldPosToScreenPos(vNeckPos, screenNeck))
					{
						float dx = screenHead.x - screenNeck.x;
						float dy = screenHead.y - screenNeck.y;
						headRadius = sqrtf(dx * dx + dy * dy) * 0.7f;
						bHasHeadPos = true;
					}
				}
			}
		}
	}

	// Draw bones
	for (int i = 0; i < pHdr->numbones; i++)
	{
		mstudiobone_t* pBone = pHdr->pBone(i);
		if (!pBone || pBone->parent == -1 || !(pBone->flags & BONE_USED_BY_HITBOX))
			continue;

		Vector vChildPos, vParentPos;
		U::Math.VectorTransform(Vector(0, 0, 0), pBones[i], vChildPos);
		U::Math.VectorTransform(Vector(0, 0, 0), pBones[pBone->parent], vParentPos);

		Vector2D screenChild, screenParent;
		if (H::Draw.WorldPosToScreenPos(vChildPos, screenChild) && H::Draw.WorldPosToScreenPos(vParentPos, screenParent))
		{
			draw->Line(static_cast<int>(screenParent.x), static_cast<int>(screenParent.y),
					   static_cast<int>(screenChild.x), static_cast<int>(screenChild.y), outlineColor, 1.0f);
			draw->Line(static_cast<int>(screenParent.x), static_cast<int>(screenParent.y),
					   static_cast<int>(screenChild.x), static_cast<int>(screenChild.y), skeletonColor, 1.0f);
		}
	}

	// Draw head circle
	if (bHasHeadPos && headRadius > 0.0f)
	{
		Vector2D screenHead;
		if (H::Draw.WorldPosToScreenPos(vHeadPos, screenHead))
		{
			draw->Circle(static_cast<int>(screenHead.x), static_cast<int>(screenHead.y), headRadius, outlineColor, 1.0f);
			draw->Circle(static_cast<int>(screenHead.x), static_cast<int>(screenHead.y), headRadius, skeletonColor, 1.0f);
		}
	}
}

void CFeatures_ESP::RenderPlayers(IDrawInterface* draw)
{
	const int nMaxClients = (I::EngineClient->GetMaxClients() + 1);
	const int nLocalIndex = I::EngineClient->GetLocalPlayer();
	C_TFPlayer* pLocal = I::ClientEntityList->GetClientEntity(nLocalIndex)->As<C_TFPlayer*>();
	if (!pLocal) return;

	player_info_t pi;
	for (int n = 1; n < nMaxClients; n++)
	{
		C_BaseEntity* pEntity = C_BaseEntity::Instance(n);
		if (!pEntity || pEntity->IsDormant()) continue;

		C_TFPlayer* pPlayer = pEntity->As<C_TFPlayer*>();
		if (!pPlayer || pPlayer->deadflag()) continue;

		const bool bIsLocal = (n == nLocalIndex);
		if (bIsLocal && !Vars::Misc::Thirdperson && !I::Input->CAM_IsThirdPerson()) continue;
		if (bIsLocal && Vars::ESP::IgnoreLocal) continue;

		if (!bIsLocal)
		{
			const bool bIsTeammate = pPlayer->InLocalTeam();
			if (Vars::ESP::IgnoreTeammates && bIsTeammate) continue;
			if (Vars::ESP::IgnoreEnemies && !bIsTeammate) continue;
		}

		if (Vars::ESP::IgnoreFriends && I::EngineClient->GetPlayerInfo(n, &pi))
			if (Vars::CustomFriends.find(pi.friendsID) != Vars::CustomFriends.end()) continue;

		if (Vars::ESP::IgnoreCloaked && pPlayer->m_flInvisibility() >= 1.0f) continue;

		int x, y, w, h;
		const int nHealth = pPlayer->GetHealth();
		const int nMaxHealth = pPlayer->GetMaxHealth();
		if (!nHealth || !nMaxHealth || !GetDynamicBounds(pPlayer, x, y, w, h)) continue;

		float r, g, b;
		pPlayer->GetGlowEffectColor(&r, &g, &b);
		
		Color boxColor, skeletonColor;
		if (Vars::ESP::UseEnemyColors)
		{
			boxColor = Color(static_cast<int>(r * 255.0f), static_cast<int>(g * 255.0f), static_cast<int>(b * 255.0f), 255);
			skeletonColor = Color(static_cast<int>(r * 255.0f), static_cast<int>(g * 255.0f), static_cast<int>(b * 255.0f), 255);
		}
		else
		{
			boxColor = Color(static_cast<int>(Vars::Colors::ESPBoxR * 255.0f), static_cast<int>(Vars::Colors::ESPBoxG * 255.0f), static_cast<int>(Vars::Colors::ESPBoxB * 255.0f), static_cast<int>(Vars::Colors::ESPBoxA * 255.0f));
			skeletonColor = Color(static_cast<int>(Vars::Colors::SkeletonR * 255.0f), static_cast<int>(Vars::Colors::SkeletonG * 255.0f), static_cast<int>(Vars::Colors::SkeletonB * 255.0f), static_cast<int>(Vars::Colors::SkeletonA * 255.0f));
		}
		
		Color healthColor;
		if (nHealth > nMaxHealth) healthColor = Color(0, 128, 255, 255);
		else if (nHealth < 25) healthColor = Color(255, 0, 0, 255);
		else if (nHealth < 50) healthColor = Color(255, 128, 0, 255);
		else if (nHealth < 80) healthColor = Color(255, 255, 0, 255);
		else healthColor = Color(0, 255, 0, 255);

		if (Vars::ESP::PlayerBoxes)
			draw->OutlinedRect(x, y, w, h, boxColor);

		if (Vars::ESP::PlayerNames && I::EngineClient->GetPlayerInfo(n, &pi))
			draw->Text(x + w / 2, y - ESPConstants::NAME_OFFSET_Y, Color(255, 255, 255, 255), TXT_CENTERX, pi.name);

		int nDrawX = x + w + ESPConstants::INFO_OFFSET_X;
		int nDrawY = y;

		if (Vars::ESP::PlayerHealth)
		{
			char healthText[32];
			sprintf_s(healthText, "%i", nHealth);
			draw->Text(nDrawX, nDrawY, healthColor, TXT_DEFAULT, healthText);
			nDrawY += ESPConstants::INFO_LINE_HEIGHT;
		}

		if (Vars::ESP::PlayerConditions)
		{
			if (pPlayer->InCondUber()) { draw->Text(nDrawX, nDrawY, Color(255, 255, 0, 255), TXT_DEFAULT, "UBER"); nDrawY += ESPConstants::INFO_LINE_HEIGHT; }
			if (pPlayer->InCondCrit()) { draw->Text(nDrawX, nDrawY, Color(255, 255, 0, 255), TXT_DEFAULT, "CRITS"); nDrawY += ESPConstants::INFO_LINE_HEIGHT; }
			if (pPlayer->InCondBerserk()) { draw->Text(nDrawX, nDrawY, Color(255, 255, 0, 255), TXT_DEFAULT, "BERSERK"); nDrawY += ESPConstants::INFO_LINE_HEIGHT; }
			if (pPlayer->InCondHaste()) { draw->Text(nDrawX, nDrawY, Color(255, 255, 0, 255), TXT_DEFAULT, "HASTE"); nDrawY += ESPConstants::INFO_LINE_HEIGHT; }
			if (pPlayer->InCondShield()) { draw->Text(nDrawX, nDrawY, Color(255, 255, 0, 255), TXT_DEFAULT, "SHIELD"); nDrawY += ESPConstants::INFO_LINE_HEIGHT; }
		}

		if (Vars::ESP::PlayerWeapons)
		{
			C_BaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
			if (pWeapon)
			{
				std::string szWeapon = (pWeapon->GetName() + 10);
				std::transform(szWeapon.begin(), szWeapon.end(), szWeapon.begin(),
					[](unsigned char c) { return std::tolower(c); });
				draw->Text(x + w / 2, y + h + ESPConstants::WEAPON_OFFSET_Y, Color(150, 150, 150, 255), TXT_CENTERX, szWeapon.c_str());
			}
		}

		if (Vars::ESP::PlayerHealthBar)
		{
			const float flMaxHealth = static_cast<float>(nMaxHealth);
			const float flHealth = U::Math.Clamp<float>(static_cast<float>(nHealth), 1.0f, flMaxHealth);
			const int nHeight = h + 1;
			const float ratio = (flHealth / flMaxHealth);
			const int barHeight = static_cast<int>(nHeight * ratio);
			const int barY = y + nHeight - barHeight;
			
			const int barX = x - ESPConstants::HEALTHBAR_OFFSET;
			const int outlineThickness = ESPConstants::HEALTHBAR_OUTLINE_THICKNESS;
			
			// Draw background
			draw->FilledRect(barX, y, ESPConstants::HEALTHBAR_WIDTH, nHeight, ESPConstants::COLOR_HEALTHBAR_BG);
			// Draw health bar
			draw->FilledRect(barX, barY, ESPConstants::HEALTHBAR_WIDTH, barHeight, healthColor);
			// Draw outline
			draw->Line(barX - outlineThickness, y - outlineThickness, barX - outlineThickness, y + nHeight + outlineThickness, ESPConstants::COLOR_OUTLINE, 1.0f);
			draw->Line(barX + ESPConstants::HEALTHBAR_WIDTH + outlineThickness, y - outlineThickness, barX + ESPConstants::HEALTHBAR_WIDTH + outlineThickness, y + nHeight + outlineThickness, ESPConstants::COLOR_OUTLINE, 1.0f);
			draw->Line(barX - outlineThickness, y - outlineThickness, barX + ESPConstants::HEALTHBAR_WIDTH + outlineThickness, y - outlineThickness, ESPConstants::COLOR_OUTLINE, 1.0f);
			draw->Line(barX - outlineThickness, y + nHeight + outlineThickness, barX + ESPConstants::HEALTHBAR_WIDTH + outlineThickness, y + nHeight + outlineThickness, ESPConstants::COLOR_OUTLINE, 1.0f);
		}

		if (Vars::ESP::PlayerSkeleton)
			RenderPlayerSkeleton(draw, pPlayer, skeletonColor);
	}
}

void CFeatures_ESP::RenderWorldItems(IDrawInterface* draw)
{
	const int nMaxClients = (I::EngineClient->GetMaxClients() + 1);
	const int nMaxEntities = I::ClientEntityList->GetMaxEntities();

	for (int n = nMaxClients; n < nMaxEntities; n++)
	{
		C_BaseEntity* pEntity = C_BaseEntity::Instance(n);
		if (!pEntity || pEntity->IsDormant()) continue;

		ClientClass* pCC = pEntity->GetClientClass();
		if (!pCC) continue;

		Vector2D screenPos;
		switch (pCC->m_ClassID)
		{
			case CTFAmmoPack:
				if (Vars::ESP::Ammo && H::Draw.WorldPosToScreenPos(pEntity->WorldSpaceCenter(), screenPos))
					draw->Text(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), Color(150, 150, 150, 255), TXT_CENTERX, "ammo");
				break;
			case CCondPowerup:
			{
				if (!Vars::ESP::Powerups) break;
				C_CondPowerup* pPowerup = pEntity->As<C_CondPowerup*>();
				if (!pPowerup || !H::Draw.WorldPosToScreenPos(pPowerup->WorldSpaceCenter(), screenPos)) break;
				const int nModelIndex = pPowerup->m_nModelIndex();
				if (m_mapPowerups.find(nModelIndex) == m_mapPowerups.end()) break;
				char powerupName[64];
				size_t convertedChars = 0;
				wcstombs_s(&convertedChars, powerupName, sizeof(powerupName), m_mapPowerups[nModelIndex], _TRUNCATE);
				draw->Text(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), Color(255, 255, 0, 255), TXT_CENTERX, powerupName);
				if (pPowerup->m_bRespawning())
				{
					char respawnText[32];
					sprintf_s(respawnText, "%.1f", pPowerup->m_flRespawnTick() - I::GlobalVarsBase->curtime);
					draw->Text(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y + 15), Color(255, 255, 0, 255), TXT_CENTERX, respawnText);
				}
				break;
			}
			case CWeaponSpawner:
			{
				if (!Vars::ESP::Weapons) break;
				C_WeaponSpawner* pSpawn = pEntity->As<C_WeaponSpawner*>();
				if (!pSpawn || !H::Draw.WorldPosToScreenPos(pSpawn->WorldSpaceCenter(), screenPos)) break;
				Color color = pSpawn->m_bSuperWeapon() ? Color(255, 255, 0, 255) : Color(150, 150, 150, 255);
				char szArray[64];
				pSpawn->GetWeaponName(szArray);
				std::string szWeapon = (szArray + 10);
				std::transform(szWeapon.begin(), szWeapon.end(), szWeapon.begin(), [](unsigned char c) { return ::tolower(c); });
				draw->Text(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), color, TXT_CENTERX, szWeapon.c_str());
				if (pSpawn->m_bRespawning())
				{
					char respawnText[32];
					sprintf_s(respawnText, "%.1fs", pSpawn->m_flRespawnTick() - I::GlobalVarsBase->curtime);
					draw->Text(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y + 15), color, TXT_CENTERX, respawnText);
				}
				break;
			}
			case CBaseAnimating:
			{
				const int nModelIndex = pEntity->m_nModelIndex();
				if (Vars::ESP::Ammo && IsAmmo(nModelIndex) && H::Draw.WorldPosToScreenPos(pEntity->WorldSpaceCenter(), screenPos))
					draw->Text(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), Color(150, 150, 150, 255), TXT_CENTERX, "ammo");
				else if (Vars::ESP::Health && IsHealth(nModelIndex) && H::Draw.WorldPosToScreenPos(pEntity->WorldSpaceCenter(), screenPos))
					draw->Text(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y), Color(0, 255, 0, 255), TXT_CENTERX, "health");
				break;
			}
		}
	}
}

// ============================================================================
// Render Entry Points
// ============================================================================


void CFeatures_ESP::Render()
{
	if (I::EngineVGui->IsGameUIVisible() || !I::EngineClient->IsInGame())
		return;

	if (!Vars::ESP::Enabled)
		return;

	ImDrawList* drawList = ImGui::GetBackgroundDrawList();
	if (!drawList)
		return;

	ImGuiDraw drawer(drawList);
	if (Vars::ESP::Players) RenderPlayers(&drawer);
	if (Vars::ESP::Items) RenderWorldItems(&drawer);
	if (Vars::Backtrack::Enabled && Vars::Backtrack::DrawSkeleton) RenderLagRecords(&drawer);
}

void CFeatures_ESP::RenderSurface()
{
	if (I::EngineVGui->IsGameUIVisible() || !I::EngineClient->IsInGame())
		return;

	if (!Vars::ESP::Enabled)
		return;

	SurfaceDraw drawer;
	if (Vars::ESP::Players) RenderPlayers(&drawer);
	if (Vars::ESP::Items) RenderWorldItems(&drawer);
	if (Vars::Backtrack::Enabled && Vars::Backtrack::DrawSkeleton) RenderLagRecords(&drawer);
}

void CFeatures_ESP::RenderLagRecords(IDrawInterface* draw)
{
	const int nMaxClients = I::EngineClient->GetMaxClients();
	const int nLocalIndex = I::EngineClient->GetLocalPlayer();
	const auto pLocal = I::ClientEntityList->GetClientEntity(nLocalIndex)->As<C_TFPlayer*>();

	if (!pLocal) return;

	for (int i = 1; i <= nMaxClients; i++)
	{
		if (i == nLocalIndex) continue;

		C_BaseEntity* pEntity = I::ClientEntityList->GetClientEntity(i)->As<C_BaseEntity*>();
		if (!pEntity || pEntity->IsDormant()) continue;

		C_TFPlayer* pPlayer = pEntity->As<C_TFPlayer*>();
		if (!pPlayer || pPlayer->deadflag()) continue;

		const auto* records = F::Backtrack.GetRecords(pPlayer->entindex());
		if (!records || records->empty()) continue;

		// Find the oldest valid record to draw (the ghost)
		const BacktrackRecord* pBestRecord = nullptr;
		for (auto it = records->rbegin(); it != records->rend(); ++it)
		{
			if (F::Backtrack.IsTickValid(it->flSimulationTime, I::GlobalVarsBase->curtime))
			{
				pBestRecord = &(*it);
				break;
			}
		}

		if (!pBestRecord) continue;
		
		// Determine color
		Color skeletonColor;
		if (Vars::ESP::UseEnemyColors)
		{
			float r, g, b;
			pPlayer->GetGlowEffectColor(&r, &g, &b);
			skeletonColor = Color(static_cast<int>(r * 255.0f), static_cast<int>(g * 255.0f), static_cast<int>(b * 255.0f), 128); // 50% alpha
		}
		else
		{
			skeletonColor = Color(static_cast<int>(Vars::Colors::SkeletonR * 255.0f), static_cast<int>(Vars::Colors::SkeletonG * 255.0f), static_cast<int>(Vars::Colors::SkeletonB * 255.0f), 128); // 50% alpha
		}
		
		// Use mutable cast to satisfy RenderPlayerSkeleton signature
		RenderPlayerSkeleton(draw, pPlayer, skeletonColor, const_cast<matrix3x4_t*>(pBestRecord->BoneMatrix));
	}
}

void CFeatures_ESP::LevelInitPostEntity()
{
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

		x = static_cast<int>(left);
		y = static_cast<int>(bottom);
		w = static_cast<int>((righ - left));
		h = static_cast<int>((top - bottom));

		return !(x > H::Draw.m_nScreenW || (x + w) < 0 || y > H::Draw.m_nScreenH || (y + h) < 0);
	}

	return false;
}
