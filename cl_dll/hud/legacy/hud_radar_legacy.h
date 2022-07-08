
#pragma once

#include "hud_sub.h"
namespace cl {
class CClientSprite;

class CHudRadarLegacy : public IBaseHudSub
{
public:
	CHudRadarLegacy();
	~CHudRadarLegacy() override;
	int VidInit(void) override;
	int Draw(float time) override;
	int GetRadarSize()
	{
		return m_hRadarOpaque.rect.right - m_hRadarOpaque.rect.left;
	}

private:
	CClientSprite m_hRadar;
	CClientSprite m_hRadarOpaque;
	CClientSprite m_hRadarBombTarget[2];
	CClientSprite m_hRadarSupplybox;

	cvar_t *cl_radartype;

	int InitBuiltinTextures();
	void DrawPlayerLocation();
	void DrawRadarDot(int x, int y, int r, int g, int b, int a);
	void DrawCross(int x, int y, int r, int g, int b, int a);

	// Call DrawT, DrawFlippedT or DrawRadarDot considering z value
	inline void DrawZAxis(Vector pos, int r, int g, int b, int a);

	void DrawT(int x, int y, int r, int g, int b, int a);
	void DrawFlippedT(int x, int y, int r, int g, int b, int a);
	bool HostageFlashTime(float flTime, struct hostage_info_t *pplayer);
	bool FlashTime(float flTime, struct extra_player_info_t *pplayer);
	Vector WorldToRadar(const Vector vPlayerOrigin, const Vector vObjectOrigin, const Vector vAngles);
	inline void DrawColoredTexture(int x, int y, int size, byte r, byte g, byte b, byte a, int texHandle);

	bool bUseRenderAPI, bTexturesInitialized;
	int hDot, hCross, hT, hFlippedT;
	int iMaxRadius;

	UniqueTexture m_iMapTitleBG;
};
}