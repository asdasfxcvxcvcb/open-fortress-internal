#pragma once

#include "../../SDK/SDK.h"
#include <unordered_map>

// Forward declarations
struct ImDrawList;

// ESP Rendering System with unified abstraction layer
// Add new ESP features once in RenderPlayers() and they work for both rendering modes

class CFeatures_ESP
{
public:
	void Render(); // ImGui/DirectX rendering
	void RenderSurface(); // ISurface/in-game rendering
	void LevelInitPostEntity();

private:
	// Drawing abstraction interface
	class IDrawInterface
	{
	public:
		virtual ~IDrawInterface() = default;
		virtual void Line(int x1, int y1, int x2, int y2, const Color& color, float thickness = 1.0f) = 0;
		virtual void OutlinedRect(int x, int y, int w, int h, const Color& color) = 0;
		virtual void FilledRect(int x, int y, int w, int h, const Color& color) = 0;
		virtual void Circle(int x, int y, float radius, const Color& color, float thickness = 1.0f) = 0;
		virtual void Text(int x, int y, const Color& color, int align, const char* text) = 0;
	};

	class ImGuiDraw : public IDrawInterface
	{
	private:
		ImDrawList* m_pDrawList;
	public:
		ImGuiDraw(ImDrawList* drawList) : m_pDrawList(drawList) {}
		void Line(int x1, int y1, int x2, int y2, const Color& color, float thickness = 1.0f) override;
		void OutlinedRect(int x, int y, int w, int h, const Color& color) override;
		void FilledRect(int x, int y, int w, int h, const Color& color) override;
		void Circle(int x, int y, float radius, const Color& color, float thickness = 1.0f) override;
		void Text(int x, int y, const Color& color, int align, const char* text) override;
	};

	class SurfaceDraw : public IDrawInterface
	{
	public:
		void Line(int x1, int y1, int x2, int y2, const Color& color, float thickness = 1.0f) override;
		void OutlinedRect(int x, int y, int w, int h, const Color& color) override;
		void FilledRect(int x, int y, int w, int h, const Color& color) override;
		void Circle(int x, int y, float radius, const Color& color, float thickness = 1.0f) override;
		void Text(int x, int y, const Color& color, int align, const char* text) override;
	};

	// Unified rendering - add new ESP features here
	void RenderPlayers(IDrawInterface* draw);
	void RenderPlayerSkeleton(IDrawInterface* draw, C_TFPlayer* pPlayer, const Color& customColor, matrix3x4_t* pBoneMatrix = nullptr);
	void RenderLagRecords(IDrawInterface* draw);
	void RenderWorldItems(IDrawInterface* draw);

	bool IsAmmo(const int nModelIndex);
	bool IsHealth(const int nModelIndex);
	bool GetDynamicBounds(C_BaseEntity* pEntity, int& x, int& y, int& w, int& h);

private:
	std::vector<int> m_vecHealth = { };
	std::vector<int> m_vecAmmo = { };
	std::map<int, const wchar_t*> m_mapPowerups = { };
	
	struct BoxCache {
		float x, y, w, h;
		float lastUpdateTime;
	};
	std::unordered_map<int, BoxCache> m_mBoxCache;
};

namespace F { inline CFeatures_ESP ESP; }