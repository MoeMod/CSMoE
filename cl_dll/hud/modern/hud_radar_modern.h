#ifndef VGUI_HUD_RADAR_H
#define VGUI_HUD_RADAR_H

#ifdef _WIN32
#pragma once
#endif

#include "hud_sub.h"
#include "com_model.h"

#include "r_texture.h"

#ifndef MAX_HOSTAGES
#define MAX_HOSTAGES 24
#endif

class CHudRadarModern : public IBaseHudSub
{
public:
	CHudRadarModern(void);

public:
	int VidInit(void) override;
	void Reset(void) override;
	void Think(void) override;
	int Draw(float time) override;
	void InitHUDData() override;

	void BuildHostageList(void);
	bool IsValidEntity(cl_entity_s *pEntity);
	bool CalcPoint(float *origin, int &screenX, int &screenY, int &scale);
	void DrawSprite(int x, int y, HSPRITE hspr, float yaw, int scale, int r, int g, int b, int a);

	bool Available() const { return m_MapSprite != nullptr; }

public:
	cvar_t *cl_newradar_size;
	cvar_t *cl_newradar_r;
	cvar_t *cl_newradar_g;
	cvar_t *cl_newradar_b;
	cvar_t *cl_newradar_a;

private:
	float m_flNextBuild;
	struct cl_entity_s *m_pHostages[MAX_HOSTAGES + 1];
	bool m_bCanRenderMapSprite;
	int m_iLastWide;

private:
	HSPRITE m_hsprPlayer;
	HSPRITE m_hsprPlayerBlue;
	HSPRITE m_hsprPlayerRed;
	HSPRITE m_hsprPlayerDead;
	HSPRITE m_hsprPlayerVIP;
	HSPRITE m_hsprPlayerC4;
	HSPRITE m_hsprBackPack;
	HSPRITE m_hsprCamera;
	HSPRITE m_hsprRadio;
	HSPRITE m_hsprC4;
	HSPRITE m_hsprHostage;

private:
	int m_HUD_radar;
	int m_HUD_radaropaque;

private:
	wrect_t *m_hrad;
	wrect_t *m_hradopaque;
	HSPRITE m_hRadar;
	HSPRITE m_hRadaropaque;

	// Thanks to crsky
	struct overview_t {
		char map[64];
		float zoom;
		float originX;
		float originY;
		bool rotated;
		char image[256];
	};
	bool LoadOverviewInfo(const char* fileName, overview_t* data);

	overview_t m_OverviewData;
	model_t *m_MapSprite;
	void LoadMapSprites(void);
};

#endif