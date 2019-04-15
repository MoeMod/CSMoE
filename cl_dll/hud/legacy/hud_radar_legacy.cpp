/*
radar.cpp - Radar
Copyright (C) 2016 a1batross

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

In addition, as a special exception, the author gives permission to
link the code of this program with the Half-Life Game Engine ("HL
Engine") and Modified Game Libraries ("MODs") developed by Valve,
L.L.C ("Valve").  You must obey the GNU General Public License in all
respects for all of the code used other than the HL Engine and MODs
from Valve.  If you modify this file, you may extend this exception
to your version of the file, but you are not obligated to do so.  If
you do not wish to do so, delete this exception statement from your
version.
*/

#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "hud_radar_legacy.h"

#include "gamemode/mods_const.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

static byte	r_RadarCross[8][8] =
{
{1,1,0,0,0,0,1,1},
{1,1,1,0,0,1,1,1},
{0,1,1,1,1,1,1,0},
{0,0,1,1,1,1,0,0},
{0,0,1,1,1,1,0,0},
{0,1,1,1,1,1,1,0},
{1,1,1,0,0,1,1,1},
{1,1,0,0,0,0,1,1}
};

static byte	r_RadarT[8][8] =
{
{1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0}
};

static byte	r_RadarFlippedT[8][8] =
{
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{0,0,0,1,1,0,0,0},
{1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1}
};

static constexpr size_t BLOCK_SIZE_MAX = 1024;
static byte	data2D[BLOCK_SIZE_MAX * 4];	// intermediate texbuffer

static void Radar_InitBitmap(int w, int h, byte *buf)
{
	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			data2D[(y * 8 + x) * 4 + 0] = 255;
			data2D[(y * 8 + x) * 4 + 1] = 255;
			data2D[(y * 8 + x) * 4 + 2] = 255;
			data2D[(y * 8 + x) * 4 + 3] = buf[y*h + x] * 255;
		}
	}
}

CHudRadarLegacy::CHudRadarLegacy()
{
	cl_radartype = CVAR_CREATE("cl_radartype", "0", FCVAR_ARCHIVE);

	bTexturesInitialized = bUseRenderAPI = false;
}

CHudRadarLegacy::~CHudRadarLegacy()
{
	// GL_FreeTexture( hDot ); engine inner texture
	if (bTexturesInitialized)
	{
		gRenderAPI.GL_FreeTexture(hT);
		gRenderAPI.GL_FreeTexture(hFlippedT);
		gRenderAPI.GL_FreeTexture(hCross);
	}
}

int CHudRadarLegacy::VidInit(void)
{
	bUseRenderAPI = g_iXash && InitBuiltinTextures();

	m_hRadar.SetSpriteByName("radar");
	m_hRadarOpaque.SetSpriteByName("radaropaque");
	m_hRadarBombTarget[0].SetSpriteByName("radar_a");
	m_hRadarBombTarget[1].SetSpriteByName("radar_b");
	m_hRadarSupplybox.SetSpriteByName("radar_item");
	iMaxRadius = (m_hRadar.rect.right - m_hRadar.rect.left) / 2.0f;
	return 1;
}

int CHudRadarLegacy::InitBuiltinTextures(void)
{
	texFlags_t defFlags = (texFlags_t)(TF_NOMIPMAP | TF_NOPICMIP | TF_NEAREST | TF_CLAMP | TF_HAS_ALPHA);

	if (bTexturesInitialized)
		return 1;

	const struct
	{
		const char	*name;
		byte	*buf;
		int		*texnum;
		int		w, h;
		void(*init)(int w, int h, byte *buf);
		int	texType;
	}
	textures[] =
	{
	{ "radarT",		   (byte*)r_RadarT,      &hT,		 8, 8, Radar_InitBitmap, TEX_CUSTOM },
	{ "radarcross",    (byte*)r_RadarCross,    &hCross,    8, 8, Radar_InitBitmap, TEX_CUSTOM },
	{ "radarflippedT", (byte*)r_RadarFlippedT, &hFlippedT, 8, 8, Radar_InitBitmap, TEX_CUSTOM }
	};
	size_t	i, num_builtin_textures = sizeof(textures) / sizeof(textures[0]);

	for (i = 0; i < num_builtin_textures; i++)
	{
		textures[i].init(textures[i].w, textures[i].h, textures[i].buf);
		*textures[i].texnum = gRenderAPI.GL_CreateTexture(textures[i].name, textures[i].w, textures[i].h, data2D, defFlags);
		if (*textures[i].texnum == 0)
		{
			for (size_t j = 0; j < i; i++)
			{
				gRenderAPI.GL_FreeTexture(*textures[i].texnum);
			}
			return 0;
		}

		gRenderAPI.GL_SetTextureType(*textures[i].texnum, textures[i].texType);
	}

	hDot = gRenderAPI.GL_LoadTexture("*white", NULL, 0, 0);

	bTexturesInitialized = true;

	return 1;
}


bool CHudRadarLegacy::FlashTime(float flTime, extra_player_info_t *pplayer)
{
	// radar flashing
	if (pplayer->radarflashes)
	{
		if (flTime > pplayer->radarflashtime)
		{
			pplayer->nextflash = !pplayer->nextflash;
			pplayer->radarflashtime += pplayer->radarflashtimedelta;
			pplayer->radarflashes--;
		}
	}
	else
	{
		return true;
	}

	return pplayer->nextflash;
}

bool CHudRadarLegacy::HostageFlashTime(float flTime, hostage_info_t *pplayer)
{
	// radar flashing
	if (pplayer->radarflashes)
	{
		if (flTime > pplayer->radarflashtime)
		{
			pplayer->nextflash = !pplayer->nextflash;
			pplayer->radarflashtime += pplayer->radarflashtimedelta;
			pplayer->radarflashes--;
		}
	}
	else
	{
		return false; // non-flashing hostage must be never drawn on radar!
	}

	return pplayer->nextflash;
}

void CHudRadarLegacy::DrawZAxis(Vector pos, int r, int g, int b, int a)
{
	const float diff = 128;

	if (pos.z > -diff && pos.z < diff)
	{
		DrawRadarDot(pos.x, pos.y, r, g, b, a);
	}
	else if (pos.z <= -diff)
	{
		// higher than player
		DrawT(pos.x, pos.y, r, g, b, a);
	}
	else
	{
		// lower than player
		DrawFlippedT(pos.x, pos.y, r, g, b, a);
	}
}

inline void CHudRadarLegacy::DrawColoredTexture(int x, int y, int size, byte r, byte g, byte b, byte a, int texHandle)
{
	gRenderAPI.GL_Bind(0, texHandle);
	gEngfuncs.pTriAPI->Color4ub(r, g, b, a);
	DrawUtils::Draw2DQuad((iMaxRadius + x - size * 2) * gHUD.m_flScale,
		(iMaxRadius + y - size * 2) * gHUD.m_flScale,
		(iMaxRadius + x + size * 2) * gHUD.m_flScale,
		(iMaxRadius + y + size * 2) * gHUD.m_flScale);
}


void CHudRadarLegacy::DrawRadarDot(int x, int y, int r, int g, int b, int a)
{
	const int size = 1;
	if (bUseRenderAPI)
	{
		DrawColoredTexture(x, y, size, r, g, b, a, hDot);
	}
	else
	{
		FillRGBA(iMaxRadius + x - size * 2, iMaxRadius + y - size * 2, size * 4, size * 4, r, g, b, a);
	}
}


void CHudRadarLegacy::DrawCross(int x, int y, int r, int g, int b, int a)
{
	const int size = 2;
	if (bUseRenderAPI)
	{
		DrawColoredTexture(x, y, size, r, g, b, a, hCross);
	}
	else
	{
		FillRGBA(iMaxRadius + x, iMaxRadius + y, size, size, r, g, b, a);
		FillRGBA(iMaxRadius + x - size, iMaxRadius + y - size, size, size, r, g, b, a);
		FillRGBA(iMaxRadius + x - size, iMaxRadius + y + size, size, size, r, g, b, a);
		FillRGBA(iMaxRadius + x + size, iMaxRadius + y - size, size, size, r, g, b, a);
		FillRGBA(iMaxRadius + x + size, iMaxRadius + y + size, size, size, r, g, b, a);
	}
}

void CHudRadarLegacy::DrawT(int x, int y, int r, int g, int b, int a)
{
	const int size = 2;

	if (bUseRenderAPI)
	{
		DrawColoredTexture(x, y, size, r, g, b, a, hT);
	}
	else
	{
		FillRGBA(iMaxRadius + x - size, iMaxRadius + y - size, size * 3, size, r, g, b, a);
		FillRGBA(iMaxRadius + x, iMaxRadius + y, size, size * 2, r, g, b, a);
	}
}

void CHudRadarLegacy::DrawFlippedT(int x, int y, int r, int g, int b, int a)
{
	const int size = 2;
	if (bUseRenderAPI)
	{
		DrawColoredTexture(x, y, size, r, g, b, a, hFlippedT);
	}
	else
	{
		FillRGBA(iMaxRadius + x, iMaxRadius + y - size, size, size * 2, r, g, b, a);
		FillRGBA(iMaxRadius + x - size, iMaxRadius + y + size, size * 3, size, r, g, b, a);
	}
}

Vector CHudRadarLegacy::WorldToRadar(const Vector vPlayerOrigin, const Vector vObjectOrigin, const Vector vAngles)
{
	Vector2D diff = vObjectOrigin.Make2D() - vPlayerOrigin.Make2D();
	const float RADAR_SCALE = 32.0f;

	// Supply epsilon values to avoid divide-by-zero
	if (diff.x == 0)
		diff.x = 0.00001f;
	if (diff.y == 0)
		diff.y = 0.00001f;

	float flOffset = DEG2RAD(vAngles.y - RAD2DEG(atan2(diff.y, diff.x)));

	// this magic 32.0f just scales position on radar
	float iRadius = min(diff.Length() / RADAR_SCALE, (float)iMaxRadius);

	// transform origin difference to radar source
	Vector ret((float)(iRadius * sin(flOffset)),
		(float)(iRadius * -cos(flOffset)),
		(float)(vPlayerOrigin.z - vObjectOrigin.z));

	return ret;
}

void CHudRadarLegacy::DrawPlayerLocation()
{
	DrawUtils::DrawConsoleString(30, 30, g_PlayerExtraInfo[gHUD.m_Scoreboard.m_iPlayerNum].location);
}

int CHudRadarLegacy::Draw(float flTime)
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) ||
		gEngfuncs.IsSpectateOnly() ||
		!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))) ||
		gHUD.m_fPlayerDead)
		return 1;

	int iTeamNumber = g_PlayerExtraInfo[gHUD.m_Scoreboard.m_iPlayerNum].teamnumber;
	int r, g, b;

	if (cl_radartype->value)
	{
		SPR_Set(m_hRadarOpaque.spr, 200, 200, 200);
		SPR_DrawHoles(0, 0, 0, &m_hRadarOpaque.rect);
	}
	else
	{
		SPR_Set(m_hRadar.spr, 25, 75, 25);
		SPR_DrawAdditive(0, 0, 0, &m_hRadarOpaque.rect);
	}

	if (strlen(g_szLocation))
	{
		int iLength, iHeight;
		gEngfuncs.pfnDrawSetTextColor(0.0f, 0.8f, 0.0f);
		gEngfuncs.pfnDrawConsoleStringLen(g_szLocation, &iLength, &iHeight);
		gEngfuncs.pfnDrawConsoleString(64 - iLength / 2, m_hRadarOpaque.rect.bottom + iHeight, g_szLocation);
	}

	if (bUseRenderAPI)
	{
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		gEngfuncs.pTriAPI->CullFace(TRI_NONE);
		gEngfuncs.pTriAPI->Brightness(1);
		gRenderAPI.GL_SelectTexture(0);
	}

	for (int i = 0; i < 33; i++)
	{
		// skip local player and dead players
		if (i == gHUD.m_Scoreboard.m_iPlayerNum || g_PlayerExtraInfo[i].dead)
			continue;

		// skip non-teammates
		if (g_PlayerExtraInfo[i].teamnumber != iTeamNumber)
			continue;

		// decide should player draw at this time. For flashing.
		// Always true for non-flashing players
		if (!FlashTime(flTime, &g_PlayerExtraInfo[i]))
			continue;

		// player with C4 must be red
		if (g_PlayerExtraInfo[i].has_c4)
		{
			DrawUtils::UnpackRGB(r, g, b, RGB_REDISH);
		}
		else
		{
			// white
			DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);
		}

		// calc radar position
		Vector pos = WorldToRadar(gHUD.m_vecOrigin, g_PlayerExtraInfo[i].origin, gHUD.m_vecAngles);

		DrawZAxis(pos, r, g, b, 255);
	}

	// Terrorist specific code( C4 Bomb )
	if (g_PlayerExtraInfo[gHUD.m_Scoreboard.m_iPlayerNum].teamnumber == TEAM_TERRORIST)
	{
		if (!g_PlayerExtraInfo[33].dead &&
			g_PlayerExtraInfo[33].radarflashes &&
			FlashTime(flTime, &g_PlayerExtraInfo[33]))
		{
			Vector pos = WorldToRadar(gHUD.m_vecOrigin, g_PlayerExtraInfo[33].origin, gHUD.m_vecAngles);
			if (g_PlayerExtraInfo[33].playerclass) // bomb planted
			{
				DrawCross(pos.x, pos.y, 255, 0, 0, 255);
			}
			else
			{
				DrawZAxis(pos, 255, 0, 0, 255);
			}
		}
	}
	// Counter-Terrorist specific code( hostages )
	else if (g_PlayerExtraInfo[gHUD.m_Scoreboard.m_iPlayerNum].teamnumber == TEAM_CT)
	{
		// draw hostages for CT
		if (gHUD.m_iModRunning == MOD_ZB2 || gHUD.m_iModRunning == MOD_ZB3)
		{
			for (int i = 0; i < MAX_HOSTAGES; i++)
			{
				if (!g_HostageInfo[i].dead)
				{
					Vector pos = WorldToRadar(gHUD.m_vecOrigin, g_HostageInfo[i].origin, gHUD.m_vecAngles);
					// supply box
					SPR_Set(m_hRadarSupplybox.spr, 200, 200, 200);
					float x = iMaxRadius + pos.x - (m_hRadarSupplybox.rect.right - m_hRadarSupplybox.rect.left) / 2.0f, y = iMaxRadius + pos.y - (m_hRadarSupplybox.rect.bottom - m_hRadarSupplybox.rect.top) / 2.0f;
					SPR_DrawAdditive(0, x, y, &m_hRadarSupplybox.rect);
				}
			}
		}
		else
		{
			for (int i = 0; i < MAX_HOSTAGES; i++)
			{
				if (!HostageFlashTime(flTime, g_HostageInfo + i))
				{
					continue;
				}

				Vector pos = WorldToRadar(gHUD.m_vecOrigin, g_HostageInfo[i].origin, gHUD.m_vecAngles);

				if (g_HostageInfo[i].dead)
				{
					DrawZAxis(pos, 255, 0, 0, 255);
				}
				else
				{
					DrawZAxis(pos, 4, 25, 110, 255);
				}
			}
		}
	}

	// BombTarget
	for (int i = 0; i < gHUD.m_FollowIcon.m_iBombTargetsNum; ++i)
	{
		Vector pos = WorldToRadar(gHUD.m_vecOrigin, gHUD.m_FollowIcon.m_vecBombTargets[i], gHUD.m_vecAngles);
		SPR_Set(m_hRadarBombTarget[i].spr, 200, 200, 200);
		float x = iMaxRadius + pos.x - (m_hRadarBombTarget[i].rect.right - m_hRadarBombTarget[i].rect.left) / 2.0f, y = iMaxRadius + pos.y - (m_hRadarBombTarget[i].rect.bottom - m_hRadarBombTarget[i].rect.top) / 2.0f;
		SPR_DrawAdditive(0, x, y, &m_hRadarBombTarget[i].rect);
	}

	return 0;
}