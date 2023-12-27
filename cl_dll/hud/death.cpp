/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// death notice
//
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include "draw_util.h"
#include "player/player_model.h"
#include "vgui_controls/controls.h"
#include "vgui/ILocalize.h"

namespace cl {

float color[3];

bool g_bFirstBlood = true;

float g_fLastAssist[MAX_CLIENTS + 1][MAX_CLIENTS + 1];
int g_iDefuser, g_iPlanter, g_CWcount[MAX_CLIENTS + 1][3];
int g_lastsoldier[2];

DECLARE_MESSAGE( m_DeathNotice, DeathMsg )

enum DrawBgType
{
	DB_NONE,
	DB_KILL,
	DB_DEATH
};

struct DeathNoticeItem
{
	char szKiller[MAX_PLAYER_NAME_LENGTH*2];
	char szVictim[MAX_PLAYER_NAME_LENGTH*2];
	int iId;	// the index number of the associated sprite
	bool bSuicide;
	bool bTeamKill;
	bool bNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
	int iHeadShotId;

	DrawBgType DrawBg;
};

#define MAX_DEATHNOTICES	4
static int DEATHNOTICE_DISPLAY_TIME = 6;
static int KILLEFFECT_DISPLAY_TIME = 3;
static float KILLICON_DISPLAY_TIME = 0.75f;

#define DEATHNOTICE_TOP		32

DeathNoticeItem rgDeathNoticeList[ MAX_DEATHNOTICES + 1 ];

static std::map<std::string, BOOL> s_KnifeNamesMap;
static std::map<std::string, BOOL> s_GrenadeNamesMap;

// Kill effect
#define KILLFLAG_HEADSHOT		(1<<0)
#define KILLFLAG_FIRSTKILL		(1<<1)
#define KILLFLAG_LASTKILL		(1<<2)
#define KILLFLAG_WALLSHOT		(1<<3)
#define KILLFLAG_SHOWMARK		(1<<4)
#define KILLFLAG_HEADSHOT_GOLD	(1<<5)
#define KILLFLAG_KNIFE			(1<<6)
#define KILLFLAG_GRENADE		(1<<7)
#define KILLFLAG_REVENGE		(1<<8)
#define KILLFLAG_INFECT			(1<<9)
#define KILLFLAG_KILLZOMBIE		(1<<10)

int CHudDeathNotice :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( DeathMsg );

	HudDeathInfo().Init();

	hud_deathnotice_time = CVAR_CREATE( "hud_deathnotice_time", "6", 0 );
	m_iFlags = 0;

	return 1;
}

void CHudDeathNotice::Reset(void)
{
	m_killNums = 0;
	m_multiKills = 0;
	m_showIcon = false;
	m_showKill = false;
	m_iconIndex = 0;
	m_killEffectTime = 0;
	m_killIconTime = 0;
}

void CHudDeathNotice :: InitHUDData( void )
{
	memset( rgDeathNoticeList, 0, sizeof(rgDeathNoticeList) );
}


int CHudDeathNotice :: VidInit( void )
{
	m_HUD_d_skull = gHUD.GetSpriteIndex( "d_skull" );
	m_HUD_d_headshot = gHUD.GetSpriteIndex("d_headshot");

	m_KM_Number0 = gHUD.GetSpriteIndex("KM_Number0");
	m_KM_Number1 = gHUD.GetSpriteIndex("KM_Number1");
	m_KM_Number2 = gHUD.GetSpriteIndex("KM_Number2");
	m_KM_Number3 = gHUD.GetSpriteIndex("KM_Number3");
	m_KM_KillText = gHUD.GetSpriteIndex("KM_KillText");
	m_KM_Icon_Head = gHUD.GetSpriteIndex("KM_Icon_Head");
	m_KM_Icon_Knife = gHUD.GetSpriteIndex("KM_Icon_knife");
	m_KM_Icon_Frag = gHUD.GetSpriteIndex("KM_Icon_Frag");

	R_InitTexture(m_csgo_killBg[0], "resource/hud/csgo/DeathNotice/KillBg_left");
	R_InitTexture(m_csgo_killBg[1], "resource/hud/csgo/DeathNotice/KillBg_center");
	R_InitTexture(m_csgo_killBg[2], "resource/hud/csgo/DeathNotice/KillBg_right");

	R_InitTexture(m_NewHud_killBg[0], "resource/Hud/DeathNotice/killbg_left_new");
	R_InitTexture(m_NewHud_killBg[1], "resource/Hud/DeathNotice/killbg_center_new");
	R_InitTexture(m_NewHud_killBg[2], "resource/Hud/DeathNotice/killbg_right_new");

	R_InitTexture(m_killBg[0], "resource/Hud/DeathNotice/KillBg_left");
	R_InitTexture(m_killBg[1], "resource/Hud/DeathNotice/KillBg_center");
	R_InitTexture(m_killBg[2], "resource/Hud/DeathNotice/KillBg_right");

	R_InitTexture(m_NewHud_deathBg[0], "resource/Hud/DeathNotice/deathbg_left_new");
	R_InitTexture(m_NewHud_deathBg[1], "resource/Hud/DeathNotice/deathbg_center_new");
	R_InitTexture(m_NewHud_deathBg[2], "resource/Hud/DeathNotice/deathbg_right_new");

	R_InitTexture(m_deathBg[0], "resource/Hud/DeathNotice/DeathBg_left");
	R_InitTexture(m_deathBg[1], "resource/Hud/DeathNotice/DeathBg_center");
	R_InitTexture(m_deathBg[2], "resource/Hud/DeathNotice/DeathBg_right");

	R_InitTexture(m_csgo_defaultBg[0], "resource/hud/csgo/DeathNotice/DefaultBg_left");
	R_InitTexture(m_csgo_defaultBg[1], "resource/hud/csgo/DeathNotice/DefaultBg_center");
	R_InitTexture(m_csgo_defaultBg[2], "resource/hud/csgo/DeathNotice/DefaultBg_right");
	
	NewAlarm().VidInit();
	HudDeathInfo().VidInit();

	s_KnifeNamesMap["d_knife"] = TRUE;
	s_KnifeNamesMap["d_dgaxe"] = TRUE;
	s_KnifeNamesMap["d_knifedragon"] = TRUE;
	s_KnifeNamesMap["d_dragonsword"] = TRUE;
	s_KnifeNamesMap["d_dualsword"] = TRUE;
	s_KnifeNamesMap["d_z4b_facelessvoid"] = TRUE;
	s_KnifeNamesMap["d_z4b_frequency9"] = TRUE;
	s_KnifeNamesMap["d_katana"] = TRUE;
	s_KnifeNamesMap["d_lance"] = TRUE;
	s_KnifeNamesMap["d_z4b_lc_sword"] = TRUE;
	s_KnifeNamesMap["d_z4b_nataknifedx"] = TRUE;
	s_KnifeNamesMap["d_z4b_oppovivo"] = TRUE;
	s_KnifeNamesMap["d_skullaxe"] = TRUE;
	s_KnifeNamesMap["d_z4b_stormgiantx"] = TRUE;
	s_KnifeNamesMap["d_z4b_strongknifex"] = TRUE;
	s_KnifeNamesMap["d_stormgiant"] = TRUE;
	s_KnifeNamesMap["d_natakinfe"] = TRUE;
	s_KnifeNamesMap["d_natakinfed"] = TRUE;
	s_KnifeNamesMap["d_thanatos9"] = TRUE;
	s_KnifeNamesMap["d_skullt9"] = TRUE;
	s_KnifeNamesMap["d_runeblade"] = TRUE;
	s_KnifeNamesMap["d_janus9"] = TRUE;
	s_KnifeNamesMap["d_summonknife"] = TRUE;
	s_KnifeNamesMap["d_balrog9"] = TRUE;
	s_KnifeNamesMap["d_holysword"] = TRUE;
	s_KnifeNamesMap["d_y22s1holyswordmb"] = TRUE;
	s_KnifeNamesMap["d_magicknife"] = TRUE;
	s_KnifeNamesMap["d_whipsword"] = TRUE;
	s_KnifeNamesMap["d_z4b_dualkukri"] = TRUE;
	s_KnifeNamesMap["d_z4b_dualslayer"] = TRUE;
	s_KnifeNamesMap["d_jknife"] = TRUE;
	s_KnifeNamesMap["d_combat"] = TRUE;
	s_KnifeNamesMap["d_mastercombat"] = TRUE;
	s_KnifeNamesMap["d_hdagger"] = TRUE;
	s_KnifeNamesMap["d_axe"] = TRUE;
	s_KnifeNamesMap["d_hammer"] = TRUE;
	s_KnifeNamesMap["d_z4b_holyfist"] = TRUE;
	s_KnifeNamesMap["d_z4b_dualstinger"] = TRUE;
	s_KnifeNamesMap["d_z4b_heartstimulater"] = TRUE;
	s_KnifeNamesMap["d_y22s2sfsword"] = TRUE;
	s_KnifeNamesMap["d_armtorch"] = TRUE;
	s_KnifeNamesMap["d_z4b_tiga"] = TRUE;
	s_KnifeNamesMap["d_y22s3dragonsword"] = TRUE;
	s_KnifeNamesMap["d_monkeywpnset3"] = TRUE;
	s_KnifeNamesMap["d_ozwpnset3"] = TRUE;
	s_KnifeNamesMap["d_tomahawk"] = TRUE;
	s_KnifeNamesMap["d_crowbarcraft"] = TRUE;
	s_KnifeNamesMap["d_dragontail"] = TRUE;
	s_KnifeNamesMap["d_katanad"] = TRUE;
	s_KnifeNamesMap["d_vulcanus9"] = TRUE;
	s_KnifeNamesMap["d_sfsword"] = TRUE;
	s_KnifeNamesMap["d_spknife"] = TRUE;
	s_KnifeNamesMap["d_crow9"] = TRUE;
	s_KnifeNamesMap["d_turbulent9"] = TRUE;
	s_KnifeNamesMap["d_ironfan"] = TRUE;
	s_KnifeNamesMap["d_swordbombard"] = TRUE;
	s_KnifeNamesMap["d_z4b_mechanichand"] = TRUE;
	s_KnifeNamesMap["d_z4b_shovelex"] = TRUE;
	s_KnifeNamesMap["d_z4b_kukri"] = TRUE;
	s_KnifeNamesMap["d_z4b_classickukri"] = TRUE;
	s_KnifeNamesMap["d_z4b_knife"] = TRUE;
	s_KnifeNamesMap["d_z4b_axe"] = TRUE;
	s_KnifeNamesMap["d_z4b_hawlchicken"] = TRUE;
	s_KnifeNamesMap["d_z4b_shovel"] = TRUE;
	s_KnifeNamesMap["d_z4b_knuckle"] = TRUE;
	s_KnifeNamesMap["d_z4b_bcaxevip"] = TRUE;
	s_KnifeNamesMap["d_z4b_btfknife"] = TRUE;
	s_KnifeNamesMap["d_z4b_balisong_luminousedge"] = TRUE;
	s_KnifeNamesMap["d_tyrantmace"] = TRUE;
	s_KnifeNamesMap["d_dvhammer"] = TRUE;
	s_KnifeNamesMap["d_hzknife"] = TRUE;
	s_KnifeNamesMap["d_dgaxeex"] = TRUE;

	s_GrenadeNamesMap["d_grenade"] = TRUE;
	s_GrenadeNamesMap["d_m24grenade"] = TRUE;
	s_GrenadeNamesMap["d_fgrenade2"] = TRUE;
	s_GrenadeNamesMap["d_firebomb"] = TRUE;
	s_GrenadeNamesMap["d_eventbomb"] = TRUE;
	s_GrenadeNamesMap["d_mooncake"] = TRUE;
	s_GrenadeNamesMap["d_pumpkin"] = TRUE;
	s_GrenadeNamesMap["d_cartfrag"] = TRUE;
	s_GrenadeNamesMap["d_holybomb"] = TRUE;
	s_GrenadeNamesMap["d_sfgrenade"] = TRUE;
	s_GrenadeNamesMap["d_chaingren"] = TRUE;
	s_GrenadeNamesMap["d_sbmine"] = TRUE;
	s_GrenadeNamesMap["d_claymore"] = TRUE;
	s_GrenadeNamesMap["d_stickybomb"] = TRUE;
	s_GrenadeNamesMap["d_holybomb_ex"] = TRUE;
	s_GrenadeNamesMap["d_y20s1grenade"] = TRUE;
	s_GrenadeNamesMap["d_waterbomb"] = TRUE;
	s_GrenadeNamesMap["d_chinaeventbomb"] = TRUE;
	s_GrenadeNamesMap["d_sfturret"] = TRUE;
	//s_GrenadeNamesMap["d_bunkerbuster"] = TRUE;
	s_GrenadeNamesMap["d_patroldrone"] = TRUE;
	//s_GrenadeNamesMap["d_divinetitan"] = TRUE;
	s_GrenadeNamesMap["d_ignitebomb"] = TRUE;
	s_GrenadeNamesMap["d_z4b_icegrenade"] = TRUE;

	return 1;
}

void CHudDeathNotice::Shutdown(void)
{
	std::fill(std::begin(m_killBg), std::end(m_killBg), nullptr);
	std::fill(std::begin(m_deathBg), std::end(m_deathBg), nullptr);

	NewAlarm().Shutdown();
}

int CHudDeathNotice :: Draw( float flTime )
{
	int x, y, r, g, b, i;

	for( i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if ( rgDeathNoticeList[i].iId == 0 )
			break;  // we've gone through them all

		if ( rgDeathNoticeList[i].flDisplayTime < flTime )
		{ // display time has expired
			// remove the current item from the list
			memmove( &rgDeathNoticeList[i], &rgDeathNoticeList[i+1], sizeof(DeathNoticeItem) * (MAX_DEATHNOTICES - i) );
			i--;  // continue on the next item;  stop the counter getting incremented
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = min( rgDeathNoticeList[i].flDisplayTime, flTime + DEATHNOTICE_DISPLAY_TIME );

		// Hide when scoreboard drawing. It will break triapi
		//if ( gViewPort && gViewPort->AllowedToPrintText() )
		//if ( !gHUD.m_iNoConsolePrint )
		{
			// Draw the death notice
			if( !g_iUser1 )
			{
				y = YRES(DEATHNOTICE_TOP) + 2 + (20 * i);  //!!!
			}
			else
			{
				y = ScreenHeight / 5 + 2 + (20 * i);
			}

			int id = (rgDeathNoticeList[i].iId == -1) ? m_HUD_d_skull : rgDeathNoticeList[i].iId;
			x = ScreenWidth - DrawUtils::ConsoleStringLen(rgDeathNoticeList[i].szVictim) - (gHUD.GetSpriteRect(id).right - gHUD.GetSpriteRect(id).left) - (YRES(5) * 3);
			if( rgDeathNoticeList[i].iHeadShotId )
				x -= gHUD.GetSpriteRect(m_HUD_d_headshot).right - gHUD.GetSpriteRect(m_HUD_d_headshot).left;

			int xMin = x, xOffset = 3;
			if (!rgDeathNoticeList[i].bSuicide)
				xMin -= (5 + DrawUtils::ConsoleStringLen(rgDeathNoticeList[i].szKiller));

			SharedTexture (*DrawBg)[3] = nullptr;
			switch (rgDeathNoticeList[i].DrawBg)
			{
			case DB_KILL:
				if (gHUD.m_hudstyle->value == 2)
					DrawBg = &m_NewHud_killBg;
				else
					DrawBg = &m_killBg; break;
			case DB_DEATH:
				if (gHUD.m_hudstyle->value == 2)
					DrawBg = &m_NewHud_deathBg;
				else
					DrawBg = &m_deathBg; break;
			default:
			{
				if (gHUD.m_hudstyle->value == 1)
				{
					DrawBg = &m_csgo_defaultBg; break;
				}
				else
				{
					DrawBg = nullptr; break;
				}
			}
			}

			if (DrawBg)
			{
				if((*DrawBg)[0])
				{
					(*DrawBg)[0]->Draw2DQuadScaled(xMin - 3 - xOffset, y, xMin - 3 - xOffset + 3, y + 16);
				}

				if ((*DrawBg)[1])
				{
					(*DrawBg)[1]->Draw2DQuadScaled(xMin - 3 - xOffset + 3, y, ScreenWidth - (YRES(5) * 3), y + 16);
				}


				if ((*DrawBg)[2])
				{
					(*DrawBg)[2]->Draw2DQuadScaled(ScreenWidth - (YRES(5) * 3), y, ScreenWidth - (YRES(5) * 3) + 3, y + 16);
				}
			}

			if ( !rgDeathNoticeList[i].bSuicide )
			{
				x -= (5 + DrawUtils::ConsoleStringLen( rgDeathNoticeList[i].szKiller ) );

				// Draw killers name
				if ( rgDeathNoticeList[i].KillerColor )
					DrawUtils::SetConsoleTextColor( rgDeathNoticeList[i].KillerColor[0], rgDeathNoticeList[i].KillerColor[1], rgDeathNoticeList[i].KillerColor[2] );

				x = 5 + DrawUtils::DrawConsoleString( x, y, rgDeathNoticeList[i].szKiller );
			}


			r = 255;  g = 80;	b = 0;
			if ( rgDeathNoticeList[i].bTeamKill )
			{
				r = 10;	g = 240; b = 10;  // display it in sickly green
			}

			// Draw death weapon
			if (gHUD.m_hudstyle->value == 2)
			{
				r = 255;
				g = 255;
				b = 255;
			}
			SPR_Set( gHUD.GetSprite(id), r, g, b );
			SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect(id) );

			x += (gHUD.GetSpriteRect(id).right - gHUD.GetSpriteRect(id).left);

			if( rgDeathNoticeList[i].iHeadShotId)
			{
				SPR_Set( gHUD.GetSprite(m_HUD_d_headshot), r, g, b );
				SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect(m_HUD_d_headshot));
				x += (gHUD.GetSpriteRect(m_HUD_d_headshot).right - gHUD.GetSpriteRect(m_HUD_d_headshot).left);
			}

			// Draw victims name (if it was a player that was killed)
			if (!rgDeathNoticeList[i].bNonPlayerKill)
			{
				if ( rgDeathNoticeList[i].VictimColor )
					DrawUtils::SetConsoleTextColor( rgDeathNoticeList[i].VictimColor[0], rgDeathNoticeList[i].VictimColor[1], rgDeathNoticeList[i].VictimColor[2] );
				x = DrawUtils::DrawConsoleString( x, y, rgDeathNoticeList[i].szVictim );
			}
		}
	}

	if (m_showKill && gHUD.m_alarmstyle->value == 1)
	{
		m_killEffectTime = min(m_killEffectTime, gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME);

		if (gHUD.m_flTime < m_killEffectTime)
		{
			int r = 255, g = 255, b = 255;
			float alpha = 3.0f * (m_killEffectTime - gHUD.m_flTime) / KILLEFFECT_DISPLAY_TIME;
			if (alpha > 1) alpha = 1.0f;
			int numIndex = -1;

			if (alpha > 0)
			{
				r *= alpha;
				g *= alpha;
				b *= alpha;

				switch (m_multiKills)
				{
				case 1:
				{
					numIndex = m_KM_Number0;
					break;
				}

				case 2:
				{
					numIndex = m_KM_Number1;
					break;
				}

				case 3:
				{
					numIndex = m_KM_Number2;
					break;
				}

				case 4:
				{
					numIndex = m_KM_Number3;
					break;
				}
				}

				if (numIndex != -1)
				{
					int numWidth, numHeight;
					int textWidth, textHeight;
					int iconWidth, iconHeight;

					numWidth = gHUD.GetSpriteRect(numIndex).right - gHUD.GetSpriteRect(numIndex).left;
					numHeight = gHUD.GetSpriteRect(numIndex).bottom - gHUD.GetSpriteRect(numIndex).top;
					textWidth = gHUD.GetSpriteRect(m_KM_KillText).right - gHUD.GetSpriteRect(m_KM_KillText).left;
					textHeight = gHUD.GetSpriteRect(m_KM_KillText).bottom - gHUD.GetSpriteRect(m_KM_KillText).top;
					iconWidth = gHUD.GetSpriteRect(m_KM_Icon_Head).right - gHUD.GetSpriteRect(m_KM_Icon_Head).left;
					iconHeight = gHUD.GetSpriteRect(m_KM_Icon_Head).bottom - gHUD.GetSpriteRect(m_KM_Icon_Head).top;

					int fix = 0;
					if (m_multiKills == 1)
						fix = 10;

					x = (50.0f * 0.01f * ScreenWidth) - (numHeight + textWidth) * 0.5f;
					y = (20.0f * 0.01f * ScreenHeight);

					SPR_Set(gHUD.GetSprite(numIndex), r, g, b);
					SPR_DrawAdditive(0, x + fix, y - numHeight / 1.8, &gHUD.GetSpriteRect(numIndex));

					SPR_Set(gHUD.GetSprite(m_KM_KillText), r, g, b);
					SPR_DrawAdditive(0, x + numWidth + 3 * fix, y - textHeight / 2, &gHUD.GetSpriteRect(m_KM_KillText));

					x = (50.0f * 0.01f * ScreenWidth) - (iconWidth) * 0.5f;
					y = y + iconHeight / 8;

					m_killIconTime = min(m_killIconTime, gHUD.m_flTime + KILLICON_DISPLAY_TIME);

					if (m_showIcon)
					{
						alpha = 2.0f * (m_killIconTime - gHUD.m_flTime) / KILLICON_DISPLAY_TIME;
						if (alpha > 1) alpha = 1.0f;

						if (alpha > 0)
						{
							r *= alpha;
							g *= alpha;
							b *= alpha;

							switch (m_iconIndex)
							{
							case 1:
							{
								SPR_Set(gHUD.GetSprite(m_KM_Icon_Head), r, g, b);
								SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_KM_Icon_Head));
								break;
							}

							case 2:
							{
								SPR_Set(gHUD.GetSprite(m_KM_Icon_Knife), r, g, b);
								SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_KM_Icon_Knife));
								break;
							}

							case 3:
							{
								SPR_Set(gHUD.GetSprite(m_KM_Icon_Frag), r, g, b);
								SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_KM_Icon_Frag));
								break;
							}
							}
						}
					}
				}
			}
		}
		else
		{
			m_showKill = false;
			m_showIcon = false;
		}
	}

	NewAlarm().RedrawAlarm(flTime);
	HudDeathInfo().Redraw();

	if( i == 0 )
		m_iFlags &= ~HUD_DRAW; // disable hud item

	return 1;
}

void CHudDeathNotice::_PlaySound(const char* sentence)
{
	std::string language = "english";
	std::string team = "GR";

	if (PlayerClassManager().GetPlayerClass(gEngfuncs.GetLocalPlayer()->index).isFemale)
	{
		if (PlayerClassManager().GetPlayerClass(gEngfuncs.GetLocalPlayer()->index).ClassID == MODEL_JPNGIRL01
			|| PlayerClassManager().GetPlayerClass(gEngfuncs.GetLocalPlayer()->index).ClassID == MODEL_JPNGIRL01ORG
			|| PlayerClassManager().GetPlayerClass(gEngfuncs.GetLocalPlayer()->index).ClassID == MODEL_CHNGIRL01)
			language = "chineseWoman";
		else
			language = "woman";
	}
	else
	{
		if (PlayerClassManager().GetPlayerClass(gEngfuncs.GetLocalPlayer()->index).ClassID == MODEL_Y20S1YEONGSIN
			|| PlayerClassManager().GetPlayerClass(gEngfuncs.GetLocalPlayer()->index).ClassID == MODEL_Y20S1LEECHANG)
			language = "chinese";
	}

	if (PlayerClassManager().GetPlayerClass(gEngfuncs.GetLocalPlayer()->index).team == TERRORIST)
		team = "BL";

	char path[96];
	snprintf(path, sizeof(path), "vox/%s/Message/%s_%s.wav", language.c_str(), sentence, team.c_str());

	HUD_EmitSound(path);
}

// This message handler may be better off elsewhere
int CHudDeathNotice :: MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf )
{
	m_iFlags |= HUD_DRAW;

	BufferReader reader( pszName, pbuf, iSize );

	int killer = reader.ReadByte();
	int victim = reader.ReadByte();
	int flag = reader.ReadShort();
	int headshot = flag & KILLFLAG_HEADSHOT;
	int multiKills = 0;
	int idx = gEngfuncs.GetLocalPlayer()->index;

	char killedwith[32];
	strncpy( killedwith, "d_", sizeof(killedwith) );
	strcat( killedwith, reader.ReadString() );
	int iMultiKill = reader.ReadByte();	//CF Msg

	//if (gViewPort)
	//	gViewPort->DeathMsg( killer, victim );
	gHUD.m_Scoreboard.DeathMsg( killer, victim );

	gHUD.m_Spectator.DeathMessage(victim);

	for (int j = 0; j < MAX_DEATHNOTICES; j++)
	{
		if (rgDeathNoticeList[j].iId == 0)
			break;

		if (rgDeathNoticeList[j].DrawBg == DB_KILL)
			multiKills++;
	}

	if (1/*cl_killmessage->value*/)
	{
		bool bSpecialKill = false;
		bool local = idx == killer ? true : false;

		if (killer != victim)
		{
			if (gHUD.IsZombieMod())
			{
				if (g_PlayerExtraInfo[victim].teamnumber == 1)
				{
					memset(g_PlayerExtraInfoEx[victim].assisttime, 0, sizeof(g_PlayerExtraInfoEx[victim].assisttime));
					memset(g_PlayerExtraInfoEx[victim].totaldmg, 0, sizeof(g_PlayerExtraInfoEx[victim].totaldmg));
				}
			}
		}
		if (g_bFirstBlood)
		{
			g_bFirstBlood = false;

			if (local)
				NewAlarm().SetAlarm(ALARM_FIRSTBLOOD);
		}

		if (local && g_PlayerExtraInfoEx[killer].revenge == victim)
			NewAlarm().SetAlarm(ALARM_PAYBACK);

		g_PlayerExtraInfoEx[killer].revenge = 0;
		g_PlayerExtraInfoEx[victim].revenge = killer;

		if (killer == idx && victim != idx)
		{
			m_killNums++;
			m_showIcon = false;

			if (headshot)
			{
				if (!multiKills)
				{
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"HeadShot\"\n");
				}
				NewAlarm().SetAlarm(ALARM_HEADSHOT);
				bSpecialKill = true;

				m_showIcon = true;
				m_iconIndex = 1;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;

				char* szSound = nullptr;
				szSound = const_cast<char*>("%!MRAD_HEADSHOT");
				gHUD.m_Radio.RadioEX(killer, szSound, PITCH_NORM);
			}

			if (s_GrenadeNamesMap.find(killedwith) != s_GrenadeNamesMap.end())
			{
				if (!multiKills)
				{
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"GotIt\"\n");
				}

				NewAlarm().SetAlarm(ALARM_GRENADE);
				bSpecialKill = true;

				m_showIcon = true;
				m_iconIndex = 3;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;

				flag |= KILLFLAG_GRENADE;
				flag &= ~KILLFLAG_INFECT;
				
			}
		}

		if (s_KnifeNamesMap.find(killedwith) != s_KnifeNamesMap.end() && !g_PlayerExtraInfo[killer].zombie)
		{
			if (killer == idx)
			{
				if (!multiKills)
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"Humililation\"\n");

				NewAlarm().SetAlarm(ALARM_KNIFE);
				bSpecialKill = true;

				m_showIcon = true;
				m_iconIndex = 2;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;

				char* szSound =nullptr;
				szSound = const_cast<char*>("%!MRAD_KNIFEKILL");
				gHUD.m_Radio.RadioEX(killer, szSound, PITCH_NORM);

				flag |= KILLFLAG_KNIFE;
			}

			if (victim == idx)
			{
				gEngfuncs.pfnClientCmd("speak \"OhNo\"\n");

				m_showIcon = true;
				m_iconIndex = 2;
				m_killIconTime = gHUD.m_flTime + KILLICON_DISPLAY_TIME;
			}
			if (!strcmp(killedwith, "d_knife") && g_PlayerExtraInfo[killer].zombie)
			{
				if (local)
				{
					NewAlarm().SetAlarm(ALARM_INFECTOR);
					bSpecialKill = true;
				}
			}
		}

		if (killer == idx && victim != idx)
		{
			if (gHUD.m_alarmstyle->value == 2)
			{
				HUD_EmitSound("vox/ui_specialkill2.wav");

				int iMainKillMark = gHUD.m_cfalarm_partmain->value;
				int iKillMarkUP = gHUD.m_cfalarm_part1->value;
				int iKillMarkEX = gHUD.m_cfalarm_part2->value;

				bool bShowExtra = iMainKillMark == KILLMARK_CLASSIC ? true : false;
				bool bKillSprite = false;

				switch (iMainKillMark)
				{
				case KILLMARK_M82A1ANGELICBEAST:
				case KILLMARK_M82A1DEMONICBEAST:
					bKillSprite = true;
				default:
					break;
				}

				if (flag & KILLFLAG_INFECT)
				{
					gHUD.m_AnimationPlayer.AddAnimInfo("InvaderMark", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
				}
				else
				{
					if (flag & KILLFLAG_HEADSHOT)
					{
						if (flag & KILLFLAG_HEADSHOT_GOLD)
							gHUD.m_NewHud.AddMark(2);
						else
							gHUD.m_NewHud.AddMark(1);
					}
					else if (flag & KILLFLAG_KNIFE)
					{
						gHUD.m_NewHud.AddMark(3);
					}
					else
					{
						gHUD.m_NewHud.AddMark(0);
					}

					switch (iMultiKill)
					{
					case 1:
					{
						if (flag & KILLFLAG_HEADSHOT)
						{
							if (flag & KILLFLAG_WALLSHOT)
							{
								if (flag & KILLFLAG_HEADSHOT_GOLD)
									gHUD.m_AnimationPlayer.AddAnimInfo("badge_headwallshot_gold", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
								else
									gHUD.m_AnimationPlayer.AddAnimInfo("badge_headwallshot", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
							}
							else
							{
								if (flag & KILLFLAG_KILLZOMBIE)
								{
									gHUD.m_AnimationPlayer.AddAnimInfo("badge_ghostshot", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
								}
								else
								{
									if (flag & KILLFLAG_HEADSHOT_GOLD)
										gHUD.m_AnimationPlayer.AddAnimInfo("badge_headshot_gold", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
									else
										gHUD.m_AnimationPlayer.AddAnimInfo("badge_headshot", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
								}
							}

							_PlaySound("Headshot");

							bShowExtra = false;
						}
						else
						{
							if (flag & KILLFLAG_KNIFE)
							{
								bShowExtra = false;
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_knife", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
								_PlaySound("Knifekill");
							}
							else if (flag & KILLFLAG_GRENADE)
							{
								bShowExtra = false;
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_grenade", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
								_PlaySound("Grenadekill");
							}
							else if (flag & KILLFLAG_WALLSHOT)
							{
								bShowExtra = false;
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_wallshot", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
							}
							else
							{
								if (flag & KILLFLAG_KILLZOMBIE)
								{
									bShowExtra = false;
									gHUD.m_AnimationPlayer.AddAnimInfo("NANO_multi1", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
								}
								else
									gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi1", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
							}
						}
						break;
					}
					case 2:
					{
						if (flag & KILLFLAG_KILLZOMBIE)
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("NANO_multi2", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
						}
						else
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi2", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);

							if (!bKillSprite)
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi2_fx", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKFX, POSITIONFLAG_KILLMARK);
						}
						
						_PlaySound("MultiKill_2");
						break;
					}
					case 3:
					{
						if (flag & KILLFLAG_KILLZOMBIE)
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("NANO_multi3", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
						}
						else
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi3", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
							if (!bKillSprite)
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi3_fx", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKFX, POSITIONFLAG_KILLMARK);
						}
						_PlaySound("MultiKill_3");
						break;
					}
					case 4:
					{
						if (flag & KILLFLAG_KILLZOMBIE)
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("NANO_multi4", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
						}
						else
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi4", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
							if (!bKillSprite)
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi4_fx", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKFX, POSITIONFLAG_KILLMARK);
						}
						_PlaySound("MultiKill_4");
						break;
					}
					case 5:
					{
						if (flag & KILLFLAG_KILLZOMBIE)
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("NANO_multi4", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
						}
						else
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi5", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
							if (!bKillSprite)
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi5_fx", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKFX, POSITIONFLAG_KILLMARK);
						}
						_PlaySound("MultiKill_5");
						break;
					}
					case 6:
					{
						if (flag & KILLFLAG_KILLZOMBIE)
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("NANO_multi4", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
						}
						else
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi6", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
							if (!bKillSprite)
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi6_fx", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKFX, POSITIONFLAG_KILLMARK);
						}
						_PlaySound("MultiKill_6");
						break;
					}
					case 7:
					{
						if (flag & KILLFLAG_KILLZOMBIE)
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("NANO_multi4", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
						}
						else
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi6", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
							if (!bKillSprite)
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi6_fx", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKFX, POSITIONFLAG_KILLMARK);
						}
						_PlaySound("MultiKill_7");
						break;
					}
					case 8:
					{
						if (flag & KILLFLAG_KILLZOMBIE)
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("NANO_multi4", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
						}
						else
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi6", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
							if (!bKillSprite)
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi6_fx", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKFX, POSITIONFLAG_KILLMARK);
						}
						_PlaySound("MultiKill_8");
						break;
					}
					default:
					{
						if (flag & KILLFLAG_KILLZOMBIE)
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("NANO_multi4", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
						}
						else
						{
							gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi6", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARK, POSITIONFLAG_KILLMARK);
							if (!bKillSprite)
								gHUD.m_AnimationPlayer.AddAnimInfo("badge_multi6_fx", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKFX, POSITIONFLAG_KILLMARK);
						}
						break;
					}
					}

					if (bKillSprite)
					{
						gHUD.m_AnimationPlayer.AddAnimInfo("Sprite", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKSPRITE, POSITIONFLAG_KILLMARK);
					}

					if (bShowExtra)
					{				
						switch (iKillMarkEX)
						{
						case 1: gHUD.m_AnimationPlayer.AddAnimInfo("badge_Elite1", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKEX, POSITIONFLAG_KILLMARK); break;
						case 2: gHUD.m_AnimationPlayer.AddAnimInfo("badge_Elite2", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKEX, POSITIONFLAG_KILLMARK); break;
						default:
							break;
						}

						switch (iKillMarkUP)
						{
						case 1: gHUD.m_AnimationPlayer.AddAnimInfo("KillMark_Upgrade1", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKUP, POSITIONFLAG_KILLMARK); break;
						case 2: gHUD.m_AnimationPlayer.AddAnimInfo("KillMark_Upgrade2", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKUP, POSITIONFLAG_KILLMARK); break;
						case 3: gHUD.m_AnimationPlayer.AddAnimInfo("KillMark_Upgrade3", ANIMATIONJSONFLAG_STATIC | ANIMATIONJSONFLAG_KILLMARKUP, POSITIONFLAG_KILLMARK); break;
						default:
							break;
						}				
					}
				}
			}
			else
			{
				switch (multiKills)
				{
				case 0:
				{
					if (!bSpecialKill)
						NewAlarm().SetAlarm(ALARM_KILL);

					m_showKill = true;
					m_multiKills = 1;
					m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
					break;
				}

				case 1:
				{
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"DoubleKill\"\n");

					NewAlarm().SetAlarm(ALARM_2KILL);

					m_showKill = true;
					m_multiKills = 2;
					m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
					break;
				}

				case 2:
				{
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"TripleKill\"\n");

					NewAlarm().SetAlarm(ALARM_3KILL);

					m_showKill = true;
					m_multiKills = 3;
					m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
					break;
				}

				default:
				{
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"MultiKill\"\n");

					NewAlarm().SetAlarm(ALARM_4KILL);

					m_showKill = true;
					m_multiKills = 4;
					m_killEffectTime = gHUD.m_flTime + KILLEFFECT_DISPLAY_TIME;
					break;
				}
				}

				switch (m_killNums)
				{
				case 5:
				{
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"Excellent\"\n");
					NewAlarm().SetAlarm(ALARM_EXCELLENT);
					break;
				}
				case 6:
				{
					if (gHUD.IsZombieMod())
						NewAlarm().SetAlarm(ALARM_LIBERATOR);
					break;
				}
				case 10:
				{
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"Incredible\"\n");
					NewAlarm().SetAlarm(ALARM_INCREDIBLE);
					break;
				}

				case 15:
				{
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"Crazy\"\n");
					NewAlarm().SetAlarm(ALARM_CRAZY);
					break;
				}

				case 20:
				{
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"CantBelive\"\n");
					NewAlarm().SetAlarm(ALARM_CANTBELIEVE);
					break;
				}

				case 25:
				{
					if (gHUD.m_alarmstyle->value == 1)
						gEngfuncs.pfnClientCmd("speak \"OutofWorld\"\n");
					NewAlarm().SetAlarm(ALARM_OUTOFWORLD);
					break;
				}
				}
			}
		}
	}

	int i;
	for ( i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if ( rgDeathNoticeList[i].iId == 0 )
			break;
	}
	if ( i == MAX_DEATHNOTICES )
	{ // move the rest of the list forward to make room for this item
		memmove( rgDeathNoticeList, rgDeathNoticeList+1, sizeof(DeathNoticeItem) * MAX_DEATHNOTICES );
		i = MAX_DEATHNOTICES - 1;
	}

	int iAssistant = 0;
	hud_player_info_t hPlayer;

	char szKiller[64], szVictim[64], szAssistant[64];
	memset(szKiller, 0, sizeof(szKiller));
	memset(szVictim, 0, sizeof(szVictim));
	memset(szAssistant, 0, sizeof(szAssistant));

	if (killer)
	{
		gEngfuncs.pfnGetPlayerInfo(killer, &hPlayer);
		strcpy(szKiller, hPlayer.name);
	}
	// Get the Victim's name
	// If victim is -1, the killer killed a specific, non-player object (like a sentrygun)
	if (((char)victim) != -1)
	{
		gEngfuncs.pfnGetPlayerInfo(victim, &hPlayer);
		strcpy(szVictim, hPlayer.name);
	}

	//if (gViewPort)
		//gViewPort->GetAllPlayersInfo();
	gHUD.m_Scoreboard.GetAllPlayersInfo();

	// Get the Killer's name
	const char *killer_name = g_PlayerInfoList[ killer ].name;
	if ( !killer_name )
	{
		killer_name = "";
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = GetClientColor( killer );
		strncpy( rgDeathNoticeList[i].szKiller, killer_name, MAX_PLAYER_NAME_LENGTH );
		rgDeathNoticeList[i].szKiller[MAX_PLAYER_NAME_LENGTH-1] = 0;
	}

	// Get the Victim's name
	const char *victim_name = NULL;
	// If victim is -1, the killer killed a specific, non-player object (like a sentrygun)
	if ( ((signed char)victim) != -1 )
		victim_name = g_PlayerInfoList[ victim ].name;
	if ( !victim_name )
	{
		victim_name = "";
		rgDeathNoticeList[i].szVictim[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = GetClientColor( victim );
		strncpy( rgDeathNoticeList[i].szVictim, victim_name, MAX_PLAYER_NAME_LENGTH );
		rgDeathNoticeList[i].szVictim[MAX_PLAYER_NAME_LENGTH-1] = 0;
	}

	// Is it a non-player object kill?
	if ( ((signed char)victim) == -1 )
	{
		rgDeathNoticeList[i].bNonPlayerKill = true;

		// Store the object's name in the Victim slot (skip the d_ bit)
		strncpy( rgDeathNoticeList[i].szVictim, killedwith+2, sizeof(killedwith) );
	}
	else
	{
		if ( killer == victim || killer == 0 )
			rgDeathNoticeList[i].bSuicide = true;

		if ( !strncmp( killedwith, "d_teammate", sizeof(killedwith)  ) )
			rgDeathNoticeList[i].bTeamKill = true;
	}

	rgDeathNoticeList[i].iHeadShotId = headshot;

	// Find the sprite in the list
	int spr = gHUD.GetSpriteIndex( killedwith );

	rgDeathNoticeList[i].iId = spr;

	rgDeathNoticeList[i].flDisplayTime = gHUD.m_flTime + hud_deathnotice_time->value;


	if (victim == idx)
		rgDeathNoticeList[i].DrawBg = DB_DEATH;
	else if (killer == idx)
		rgDeathNoticeList[i].DrawBg = DB_KILL;
	else
		rgDeathNoticeList[i].DrawBg = DB_NONE;

	if (rgDeathNoticeList[i].bNonPlayerKill)
	{
		ConsolePrint( rgDeathNoticeList[i].szKiller );
		ConsolePrint( " killed a " );
		ConsolePrint( rgDeathNoticeList[i].szVictim );
		ConsolePrint( "\n" );
	}
	else
	{
		// record the death notice in the console
		if ( rgDeathNoticeList[i].bSuicide )
		{
			ConsolePrint( rgDeathNoticeList[i].szVictim );

			if ( !strncmp( killedwith, "d_world", sizeof(killedwith)  ) )
			{
				ConsolePrint( " died" );
			}
			else
			{
				ConsolePrint( " killed self" );
			}
		}
		else if ( rgDeathNoticeList[i].bTeamKill )
		{
			ConsolePrint( rgDeathNoticeList[i].szKiller );
			ConsolePrint( " killed his teammate " );
			ConsolePrint( rgDeathNoticeList[i].szVictim );
		}
		else
		{
			if( headshot )
				ConsolePrint( "*** ");
			ConsolePrint( rgDeathNoticeList[i].szKiller );
			ConsolePrint( " killed " );
			ConsolePrint( rgDeathNoticeList[i].szVictim );

			float fDamageHighest = 0.0f;
			bool local = false;

			for (int j = 0; j < MAX_CLIENTS + 1; j++)
			{
				if (j == killer)
					continue;
				if ((g_PlayerExtraInfo[j].teamnumber == g_PlayerExtraInfo[victim].teamnumber || g_PlayerExtraInfo[j].teamnumber != g_PlayerExtraInfo[killer].teamnumber))
					continue;

				local = idx == j ? true : false;

				if (g_PlayerExtraInfoEx[j].assisttime[0][victim] > gHUD.m_flTime)
				{
					if (g_PlayerExtraInfoEx[j].totaldmg[victim] > fDamageHighest)
					{
						iAssistant = j;
						fDamageHighest = g_PlayerExtraInfoEx[j].totaldmg[victim];
					}


					if (gHUD.IsZombieMod() && local)
					{

					}

					g_PlayerExtraInfoEx[j].assist++;

					if (g_PlayerExtraInfoEx[j].assist >= 5)
					{
						g_PlayerExtraInfoEx[j].assist = 0;

						if (local)
							NewAlarm().SetAlarm(ALARM_SUPPORTER);
					}
					else if (local && !NewAlarm().IsPlaying(ALARM_KINGMAKER, false))
						NewAlarm().SetAlarm(ALARM_ASSIST);
				}
				g_PlayerExtraInfoEx[j].totaldmg[victim] = 0.0f;

				if (gHUD.IsZombieMod() && g_PlayerExtraInfo[j].zombie)
				{
					for (int k = 1; k <= 2; k++)
					{
						int target = k == 1 ? victim : killer;
						if (g_PlayerExtraInfoEx[j].assisttime[k][target] > gHUD.m_flTime)
						{
							gEngfuncs.pfnGetPlayerInfo(j, &hPlayer); // Disable default assist system
							strcpy(szAssistant, hPlayer.name);

							if (local)
							{
								//if (g_PlayerExtraInfo[killer].zombie && !g_PlayerExtraInfo[victim].zombie)
								if (g_PlayerExtraInfo[killer].zombie && !g_PlayerExtraInfo[victim].zombie)
								{
									NewAlarm().SetAlarm(ALARM_ZOMBIEBOMB);
									//g_ZombieScore.assist_infection++;
								}
							}
							//g_PlayerInfoEx[i].assisttime[k][target] = 0.0f;
						}
					}
				}
			}
			if (iAssistant)
			{
				gEngfuncs.pfnGetPlayerInfo(iAssistant, &hPlayer);
				strcpy(szAssistant, hPlayer.name);
			}

			if (szAssistant[0])
			{
				char szTemp[64];
				sprintf(szTemp, "%s + %s", szKiller, szAssistant);
				strncpy(rgDeathNoticeList[i].szKiller, szTemp, MAX_PLAYER_NAME_LENGTH);
				rgDeathNoticeList[i].szKiller[MAX_PLAYER_NAME_LENGTH - 1] = 0;
			}
			//Extra Alarm
			int first = 0, back = 0, kingmaker = 1;
			int count[2]{};
			for (int i = 0; i < MAX_CLIENTS + 1; i++)
			{
				if (g_PlayerExtraInfo[i].teamnumber == g_PlayerExtraInfo[victim].teamnumber || gHUD.m_iModRunning == MOD_DM)
				{
					if (!((g_PlayerInfoList[first].name && g_PlayerInfoList[first].name[0] != 0)))
						first = i;

					if (!((g_PlayerInfoList[back].name && g_PlayerInfoList[back].name[0] != 0)))
						back = i;

					if (g_PlayerExtraInfo[i].frags < g_PlayerExtraInfo[back].frags)
						back = i;

					if (g_PlayerExtraInfo[i].frags > g_PlayerExtraInfo[first].frags)
						first = i;
				}
				else if (gHUD.m_iModRunning == MOD_DM || g_PlayerExtraInfo[i].teamnumber == g_PlayerExtraInfo[killer].teamnumber)
				{
					if (g_PlayerExtraInfo[i].frags > g_PlayerExtraInfo[killer].frags + 1)
						kingmaker = 0;
				}
			}
			for (int i = 0; i < MAX_CLIENTS + 1; i++)
			{
				if (!((g_PlayerInfoList[i].name && g_PlayerInfoList[i].name[0] != 0)))
					continue;

				if (!g_PlayerExtraInfo[i].dead)
					count[(g_PlayerExtraInfo[i].teamnumber == 2)]++;

				if (i == victim || i == killer)
					continue;

				if (g_PlayerExtraInfo[i].teamnumber != g_PlayerExtraInfo[killer].teamnumber && gHUD.m_iModRunning != MOD_DM)
					continue;

				if (g_fLastAssist[victim][i] > gHUD.m_flTime && !g_PlayerExtraInfo[i].dead)
				{
					//if(idx == killer)
					NewAlarm().SetAlarm(ALARM_SAVIOR);
					g_fLastAssist[victim][i] = 0.0;
				}
				if (g_fLastAssist[i][victim] > gHUD.m_flTime)
				{
					if (first == victim && g_PlayerExtraInfo[first].frags)
						if (idx == i)
							NewAlarm().SetAlarm(ALARM_INVISHAND);
					if (kingmaker)
						if (idx == i)
							NewAlarm().SetAlarm(ALARM_KINGMAKER);

					//if (gHUD.m_iModRunning != MOD_DM && gHUD.m_iModRunning != MOD_TD)
						//UpdateFrags(i, 1);

					g_fLastAssist[i][victim] = 0.0;
				}
			}
			if (first == victim && g_PlayerExtraInfo[first].frags != g_PlayerExtraInfo[back].frags)
				if (idx == killer)
					NewAlarm().SetAlarm(ALARM_KINGMURDER);

			if (back == victim && g_PlayerExtraInfo[first].frags != g_PlayerExtraInfo[back].frags)
				if (idx == killer)
					NewAlarm().SetAlarm(ALARM_BACKMARKER);
			switch (g_CWcount[killer][0])
			{
			case 0:
			{
				g_CWcount[killer][0] = 1;
				if (g_CWcount[killer][1] >= 5 || g_CWcount[killer][2] >= 3)
					if (idx == killer)
						NewAlarm().SetAlarm(ALARM_WELCOME);
			}
			case 1:
			{
				if (g_CWcount[killer][1] >= 3 || g_CWcount[killer][2] >= 2)
					if (idx == killer)
						NewAlarm().SetAlarm(ALARM_COMEBACK);
			}
			}
			for (int j = 1; j <= 2; j++)
				g_CWcount[killer][j] = 0;

			if (!g_PlayerExtraInfo[victim].dead)
			{
				if (g_PlayerExtraInfo[victim].has_c4) // m_bHasC4
					if (idx == killer)
						NewAlarm().SetAlarm(ALARM_C4MANKILL);
			}
			if ((gHUD.m_Scoreboard.m_iTeamAlive_CT - 1) == 1 || (gHUD.m_Scoreboard.m_iTeamAlive_T - 1) == 1)
			{
				for (int i = 0; i < MAX_CLIENTS + 1; i++)
				{
					if (!((g_PlayerInfoList[i].name && g_PlayerInfoList[i].name[0] != 0)))
						continue;

					if (((gHUD.m_Scoreboard.m_iTeamAlive_T - 1) == 1 && g_PlayerExtraInfo[i].teamnumber == 1 && g_lastsoldier[0]) || ((gHUD.m_Scoreboard.m_iTeamAlive_CT - 1 == 1) && g_PlayerExtraInfo[i].teamnumber == 2 && !g_lastsoldier[1]))
					{
						if (idx == i)
							NewAlarm().SetAlarm(ALARM_LASTSOLDIER);
						g_lastsoldier[g_PlayerExtraInfo[i].teamnumber - 1] = 1;
						break;
					}
				}
			}
			if ((gHUD.m_Scoreboard.m_iTeamAlive_CT) == 1 || (gHUD.m_Scoreboard.m_iTeamAlive_T) == 1)
			{
				if ((gHUD.m_Scoreboard.m_iTeamAlive_CT) == 1 && g_PlayerExtraInfo[killer].teamnumber == TERRORIST)
				{
					if (!g_PlayerExtraInfo[killer].dead && idx == killer)
						NewAlarm().SetAlarm(ALARM_THELAST);
				}
				else if ((gHUD.m_Scoreboard.m_iTeamAlive_T ) == 1 && g_PlayerExtraInfo[killer].teamnumber == CT)
				{
					if (!g_PlayerExtraInfo[killer].dead && idx == killer)
						NewAlarm().SetAlarm(ALARM_THELAST);
				}
			}

			if (victim == idx)
			{
				gHUD.m_ZB2.m_flAliveTime = 0.0f;
			}
		}

		if ( *killedwith && (*killedwith > 13 ) && strncmp( killedwith, "d_world", sizeof(killedwith) ) && !rgDeathNoticeList[i].bTeamKill )
		{
			if ( headshot )
				ConsolePrint(" with a headshot from ");
			else
				ConsolePrint(" with ");

			ConsolePrint( killedwith+2 ); // skip over the "d_" part
		}

		if( headshot ) ConsolePrint( " ***");
		ConsolePrint( "\n" );
	}

	return 1;
}


}


