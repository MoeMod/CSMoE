/*
zb2_skill.cpp - CSMoE Client HUD : elements for Zombie Skills
Copyright (C) 2019 Moemod Yanase

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"

#include "zb2.h"
#include "zb2_skill.h"

#include <algorithm>
#include <functional>

namespace cl {

struct CHudZB2_Skill::Config
{
	static const char * const ZOMBIE_SKILL_HUD_ICON[MAX_ZOMBIE_SKILL];
	static const char * const ZOMBIE_SKILL_HUD_TIP[MAX_ZOMBIE_SKILL];
	//static const char * const ZOMBIE_CLASS_HUD_ICON[MAX_ZOMBIE_CLASS];
	static const char * const ZOMBIE_SKILL_HUD_ICON_NEW[MAX_ZOMBIE_SKILL];
	static const char * const ZOMBIE_CLASS_HUD_ICON_NEW[MAX_ZOMBIE_CLASS];
	static const char * const ZOMBIE_ITEM_HUD_ICON[2][3];
};

const char *  const CHudZB2_Skill::Config::ZOMBIE_SKILL_HUD_ICON[MAX_ZOMBIE_SKILL] =
{
		"", // ZOMBIE_SKILL_EMPTY
		"zombiFCT2", // ZOMBIE_SKILL_SPRINT
		"zombiICT", // ZOMBIE_SKILL_HEADSHOT
		"zombiJCT", // ZOMBIE_SKILL_KNIFE2X
		"zombicrazy", // ZOMBIE_SKILL_CRAZY,
		"zombiHiding", // ZOMBIE_SKILL_HIDE,
		"zombitrap", // ZOMBIE_SKILL_TRAP,
		"zombismoke", // ZOMBIE_SKILL_SMOKE,
		"zombiheal", // ZOMBIE_SKILL_HEAL,
		"zombideimos", // ZOMBIE_SKILL_SHOCK,
		"zombicrazy2", // ZOMBIE_SKILL_CRAZY2,
		"", // ZOMBIE_SKILL_EMPTY
		"", // ZOMBIE_SKILL_EMPTY
		"", // ZOMBIE_SKILL_EMPTY
		"", // ZOMBIE_SKILL_EMPTY
};

const char *  const CHudZB2_Skill::Config::ZOMBIE_SKILL_HUD_TIP[MAX_ZOMBIE_SKILL] =
{
		"", // ZOMBIE_SKILL_EMPTY
		"", // ZOMBIE_SKILL_SPRINT
		"", // ZOMBIE_SKILL_HEADSHOT
		"", // ZOMBIE_SKILL_KNIFE2X
		"resource/helperhud/evolution", // ZOMBIE_SKILL_CRAZY,
		"resource/helperhud/hiding", // ZOMBIE_SKILL_HIDE,
		"resource/helperhud/trap", // ZOMBIE_SKILL_TRAP,
		"resource/helperhud/smoke", // ZOMBIE_SKILL_SMOKE,
		"resource/helperhud/heal", // ZOMBIE_SKILL_HEAL,
		"resource/helperhud/tentacle", // ZOMBIE_SKILL_SHOCK,
		"resource/helperhud/crazyspeed", // ZOMBIE_SKILL_CRAZY2,
};

//const char *  const CHudZB2_Skill::Config::ZOMBIE_CLASS_HUD_ICON[MAX_ZOMBIE_CLASS] =
//{
//		"", // ZOMBIE_CLASS_HUMAN
//		"", // ZOMBIE_CLASS_TANK,
//		"zombiDTER", // ZOMBIE_CLASS_SPEED,
//		"zombiETER", // ZOMBIE_CLASS_HEAVY,
//		"zombiFTER", // ZOMBIE_CLASS_PC,
//		"zombiGTER", // ZOMBIE_CLASS_HEAL,
//		"zombiITER", // ZOMBIE_CLASS_DEIMOS,
//		"zombiJTER", // ZOMBIE_CLASS_DEIMOS2,
//		"", // ZOMBIE_CLASS_EMPTY
//		"", // ZOMBIE_CLASS_EMPTY,
//		"", // ZOMBIE_CLASS_EMPTY,
//};

const char *  const CHudZB2_Skill::Config::ZOMBIE_SKILL_HUD_ICON_NEW[MAX_ZOMBIE_SKILL] =
{
		"", // ZOMBIE_SKILL_EMPTY
		"resource/zombi/humanskill_hm_spd", // ZOMBIE_SKILL_SPRINT
		"resource/zombi/humanskill_hm_hd", // ZOMBIE_SKILL_HEADSHOT
		"resource/zombi/humanskill_hm_2x", // ZOMBIE_SKILL_KNIFE2X
		"resource/zombi/zombieskill_zombicrazy", // ZOMBIE_SKILL_CRAZY,
		"resource/zombi/zombieskill_zombihiding", // ZOMBIE_SKILL_HIDE,
		"resource/zombi/zombieskill_zombitrap", // ZOMBIE_SKILL_TRAP,
		"resource/zombi/zombieskill_zombismoke", // ZOMBIE_SKILL_SMOKE,
		"resource/zombi/zombieskill_zombiheal", // ZOMBIE_SKILL_HEAL,
		"resource/zombi/zombieskill_zombideimos", // ZOMBIE_SKILL_SHOCK,
		"resource/zombi/zombieskill_zombicrazy2", // ZOMBIE_SKILL_CRAZY2,
		"resource/zombi/zombieskill_booster",//ZOMBIE_SKILL_BOOSTER,
		"resource/zombi/zombieskill_zombiteleport",//ZOMBIE_SKILL_TELEPORT,
		"resource/zombi/zombieskill_zombiteleport2",//ZOMBIE_SKILL_TELEPORT2,
		"resource/zombi/zombieskill_zombistiffen",//ZOMBIE_SKILL_CHINA,
		"resource/zombi/zombieskill_pass",//ZOMBIE_SKILL_PASS
		"resource/zombi/zombieskill_glide",//ZOMBIE_SKILL_GLIDE
		"resource/zombi/zombieskill_dive",//ZOMBIE_SKILL_DIVE
		"resource/zombi/zombieskill_zombirevival",//ZOMBIE_SKILL_REVIVAL
		"resource/zombi/zombieskill_zombipile",//ZOMBIE_SKILL_STAMPER
		"resource/zombi/zombieskill_zombipenetration",//ZOMBIE_SKILL_PENETRATION
		"resource/zombi/zombieskill_zombijumpup",//ZOMBIE_SKILL_JUMPUP
		"resource/zombi/zombieskill_zombihook",//ZOMBIE_SKILL_HOOK
		"resource/zombi/zombieskill_jumpupm",//ZOMBIE_SKILL_JUMPUPM
		"resource/zombi/zombieskill_armorrecovery",//ZOMBIE_SKILL_ARMORRECOVERY
		"resource/zombi/zombieskill_hpbuff",//ZOMBIE_SKILL_HPBUFF
		"resource/zombi/zombieskill_strengthrecovery",//ZOMBIE_SKILL_STRENGTHRECOVERY
		"resource/zombi/zombieskill_chargeslash",//ZOMBIE_SKILL_DASH
		"resource/zombi/zombieskill_zombipileexp",//ZOMBIE_SKILL_PILEEXP
		"resource/zombi/zombieskill_charge",//ZOMBIE_SKILL_CHARGE
		"resource/zombi/zombieskill_discharge",//ZOMBIE_SKILL_DISCHARGE
		"resource/zombi/zombieskill_zombiselfdestruct",//ZOMBIE_SKILL_SELFDESTRUCT
		"resource/zombi/zombieskill_zombicreaterocket",//ZOMBIE_SKILL_CREATEROCKET
		"resource/zombi/zombieskill_webshooter",//ZOMBIE_SKILL_WEBSHOOTER
		"resource/zombi/zombieskill_webbomb",//ZOMBIE_SKILL_WEBBOMB
		"resource/zombi/zombieskill_protect",//ZOMBIE_SKILL_PROTECT
		"resource/zombi/zombieskill_dogshoot",//ZOMBIE_SKILL_DOGSHOOT
		"resource/zombi/zombieskill_rush",//ZOMBIE_SKILL_RUSH
		"resource/zombi/zombieskill_scream",//ZOMBIE_SKILL_RUSH
};

const char *  const CHudZB2_Skill::Config::ZOMBIE_CLASS_HUD_ICON_NEW[MAX_ZOMBIE_CLASS] =
{
		"", // ZOMBIE_CLASS_HUMAN
		"resource/zombi/zombietype_defaultzb", // ZOMBIE_CLASS_TANK,
		"resource/zombi/zombietype_lightzb", // ZOMBIE_CLASS_SPEED,
		"resource/zombi/zombietype_heavyzb", // ZOMBIE_CLASS_HEAVY,
		"resource/zombi/zombietype_pczb", // ZOMBIE_CLASS_PC,
		"resource/zombi/zombietype_doctorzb", // ZOMBIE_CLASS_HEAL,
		"resource/zombi/zombietype_deimoszb", // ZOMBIE_CLASS_DEIMOS,
		"resource/zombi/zombietype_deimos2zb", // ZOMBIE_CLASS_DEIMOS2,
		"resource/zombi/zombietype_deimos2zb", // ZOMBIE_CLASS_ZBS_DEIMOS2,
		"resource/zombi/zombietype_teleportzb",//ZOMBIE_CLASS_TELEPORT,
		"resource/zombi/zombietype_boosterzb",//ZOMBIE_CLASS_BOOSTER,
		"resource/zombi/zombietype_chinazb",//ZOMBIE_CLASS_CHINA,
		"resource/zombi/zombietype_passzb",//ZOMBIE_CLASS_PASS
		"resource/zombi/zombietype_flyingzb",//ZOMBIE_CLASS_FLYING
		"resource/zombi/zombietype_revivalzb",//ZOMBIE_CLASS_REVIVAL
		"resource/zombi/zombietype_undertakerzb",//ZOMBIE_CLASS_STAMPER
		"resource/zombi/zombietype_residentzb",//ZOMBIE_CLASS_RESIDENT
		"resource/zombi/zombietype_witchzb",//ZOMBIE_CLASS_WITCH
		"resource/zombi/zombietype_akshazb",//ZOMBIE_CLASS_AKSHA
		"resource/zombi/zombietype_boomerzb",//ZOMBIE_CLASS_BOOMER
		"resource/zombi/zombietype_nemesiszb",//ZOMBIE_CLASS_NEMESIS
		"resource/zombi/zombietype_spiderzb",//ZOMBIE_CLASS_SPIDER
		"resource/zombi/zombietype_deathknightzb",//ZOMBIE_CLASS_DEATHKNIGHT
		"resource/zombi/zombietype_meatwallzb",//ZOMBIE_CLASS_MEATWALL
		"resource/zombi/zombietype_zbzdarknormalzb",//ZOMBIE_CLASS_Z4NORMAL
		"resource/zombi/zombietype_zbzdarkspeedzb",//ZOMBIE_CLASS_Z4SPEED
		"resource/zombi/zombietype_zbzdarkheavyzb",//ZOMBIE_CLASS_Z4HEAVY
		"resource/zombi/zombietype_zbzdarkhiddenzb",//ZOMBIE_CLASS_Z4HIDDEN
		"resource/zombi/zombietype_zbzdarkhumpbackzb",//ZOMBIE_CLASS_Z4HUMPBACKS
		"resource/zombi/zombietype_sirenzb",//ZOMBIE_CLASS_SIREN
};

const char *  const CHudZB2_Skill::Config::ZOMBIE_ITEM_HUD_ICON[2][3] =
{
		{ "zombiACT", "zombiBCT", "zombiHCT" },
		{ "zombiATER", "zombiBTER", "zombiHTER" }
};

CHudZB2_Skill::CHudZB2_Skill(void) :  // 0-init
	m_HUD_zombirecovery(0),
	m_HUD_zombieGKey(0),
	m_HUD_SkillIcons{},
	//m_HUD_ClassIcons{},
	m_flRecoveryBeginTime(0.0f),
	m_iCurrentClass(ZOMBIE_CLASS_HUMAN),
	m_ZombieSkillHudIcons{}
{
	touch_enable = gEngfuncs.pfnGetCvarPointer("touch_enable");
}

int CHudZB2_Skill::VidInit(void)
{
	m_flRecoveryBeginTime = 0.0f;
	m_HUD_zombirecovery = gHUD.GetSpriteIndex("zombirecovery");
	m_HUD_zombieGKey = gHUD.GetSpriteIndex("zombiGkey");
	for (int i = 0; i < 12; ++i)
	{
		if(Config::ZOMBIE_SKILL_HUD_ICON[i][0] != '\0')
			m_HUD_SkillIcons[i] = gHUD.GetSpriteIndex(Config::ZOMBIE_SKILL_HUD_ICON[i]);
	}

	/*for (int i = 0; i < 8; ++i)
	{
		if (Config::ZOMBIE_CLASS_HUD_ICON[i][0] != '\0')
			m_HUD_ClassIcons[i] = gHUD.GetSpriteIndex(Config::ZOMBIE_CLASS_HUD_ICON[i]);
	}*/

	for (int i = 0; i < 11; ++i)
	{
		if (Config::ZOMBIE_SKILL_HUD_TIP[i][0] != '\0')
			if (!m_pTexture_SkillTips[i])
				m_pTexture_SkillTips[i] = R_LoadTextureUnique(Config::ZOMBIE_SKILL_HUD_TIP[i]);
	}

	if(!m_pTexture_skillslotkeybg)
		m_pTexture_skillslotkeybg = R_LoadTextureUnique("resource/zombi/skillslotkeybg");
	if (!m_pTexture_skillslotbg)
		m_pTexture_skillslotbg = R_LoadTextureUnique("resource/zombi/skillslotbg");

	for (int i = 0; i < MAX_ZOMBIE_SKILL; ++i)
	{
		if (Config::ZOMBIE_SKILL_HUD_ICON_NEW[i] && Config::ZOMBIE_SKILL_HUD_ICON_NEW[i][0] != '\0')
			if (!m_pTexture_NewSkillIcons[i])
				m_pTexture_NewSkillIcons[i] = R_LoadTextureUnique(Config::ZOMBIE_SKILL_HUD_ICON_NEW[i]);
	}

	for (int i = 0; i < MAX_ZOMBIE_CLASS; ++i)
	{
		if (Config::ZOMBIE_CLASS_HUD_ICON_NEW[i] && Config::ZOMBIE_CLASS_HUD_ICON_NEW[i][0] != '\0')
		{
			if (!m_pTexture_NewClassIcons[i])
			{
				m_pTexture_NewClassIcons[i] = R_LoadTextureShared(Config::ZOMBIE_CLASS_HUD_ICON_NEW[i]);

				if (i == 0 || i > ((int)ZOMBIE_CLASS_MEATWALL + 1))
					continue;

				if (i == ((int)ZOMBIE_CLASS_MEATWALL + 1))
				{
					//PreInit SharedTexture
					m_pTexture_RandomZbClass = R_LoadTextureShared("resource\\hud\\zombie\\zombietype_random");
					gHUD.m_ZB2.SetSelectorIcon(i - 1, "resource\\hud\\zombie\\zombietype_random");
					gHUD.m_ZB2.SetSelectorIconLevel(i - 1, 0);
				}
				else if (i == ((int)ZOMBIE_CLASS_MEATWALL))
				{
					//ZOMBIE_CLASS_SIREN
					gHUD.m_ZB2.SetSelectorIcon(i - 1, Config::ZOMBIE_CLASS_HUD_ICON_NEW[ZOMBIE_CLASS_SIREN]);
					gHUD.m_ZB2.SetSelectorIconLevel(i - 1, 0);
				}
				else
				{
					gHUD.m_ZB2.SetSelectorIcon(i - 1, Config::ZOMBIE_CLASS_HUD_ICON_NEW[i]);
					gHUD.m_ZB2.SetSelectorIconLevel(i - 1, KeeperClassInfo[i - 1].level);

					if (i == ((int)ZOMBIE_CLASS_TELEPORT) || i == ((int)ZOMBIE_CLASS_REVIVAL) || i == ((int)ZOMBIE_CLASS_NEMESIS) || i == ((int)ZOMBIE_CLASS_SPIDER) || i == ((int)ZOMBIE_CLASS_DEATHKNIGHT))
					{
						gHUD.m_ZB2.SetSelectorIconBan(i - 1);
						gHUD.m_ZB2.SetSelectorIconLevel(i - 1, 0);
					}
				}
			}
		}
	}

	return 1;
}

void CHudZB2_Skill::Reset(void)
{
	m_flRecoveryBeginTime = 0.0f;
	OnSkillInit();
}

int CHudZB2_Skill::Draw(float time)
{
	if (!gHUD.m_pCvarDraw->value)
		return 0;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	if (g_iUser1)
		return 0;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 0;

	int iHeight = gHUD.GetSpriteRect(m_HUD_zombirecovery).bottom - gHUD.GetSpriteRect(m_HUD_zombirecovery).top;
	int x = 0; // iWidth / 2;
	int y = ScreenHeight - gHUD.m_iFontHeight * 3 / 2 - iHeight;

	x = DrawHealthRecoveryIcon(time, x, y);
	if (false)
	{
		if (m_iCurrentClass >= ZOMBIE_CLASS_TELEPORT && m_iCurrentClass < MAX_ZOMBIE_CLASS)
			DrawSkillBoardNew(time);
		else
		{
			x = DrawSkillBoard(time, x, y);
			DrawSkillTip(time);
		}
	}
	else
	{
		DrawSkillBoardNew(time);
	}

	return 1;
}

void CHudZB2_Skill::Think()
{
	for (int i = 0; i < 4; ++i)
	{
		auto& icon = m_ZombieSkillHudIcons[i];
		if (icon.m_iCurrentSkillStatus == SKILL_STATUS_USING || icon.m_iCurrentSkillStatus == SKILL_STATUS_FREEZING)
		{
			if (gHUD.m_flTime > icon.m_flTimeSkillReady)
			{
				icon.m_iCurrentSkillStatus = SKILL_STATUS_READY;
				icon.m_flTimeSkillBlink = gHUD.m_flTime + 3.0f;

				char buf[128];
				sprintf(buf, "touch_show \"_moe_skill%d_button\"\n", i);
				gEngfuncs.pfnClientCmd(buf);
			}

		}
	}
}

void CHudZB2_Skill::OnHealthRecovery()
{
	// begin draw HUD
	m_flRecoveryBeginTime = gHUD.m_flTime;
}

int CHudZB2_Skill::DrawHealthRecoveryIcon(float time, int x, int y) const
{
	int iWidth = gHUD.GetSpriteRect(m_HUD_zombirecovery).right - gHUD.GetSpriteRect(m_HUD_zombirecovery).left;
	if (time > m_flRecoveryBeginTime + 1.0f)
		return x + iWidth;

	int a = 255.0f - (time - m_flRecoveryBeginTime) * 255.0f;

	int r, g, b;
	DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);
	DrawUtils::ScaleColors(r, g, b, a);

	SPR_Set(gHUD.GetSprite(m_HUD_zombirecovery), r, g, b);
	SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_zombirecovery));


	return x + iWidth;
}

int CHudZB2_Skill::DrawSkillBoard(float time, int x, int y) const
{
	for (auto &icon : m_ZombieSkillHudIcons)
	{
		x = DrawSkillIcon(time, x, y, icon);
	}
	return x;
}

int CHudZB2_Skill::DrawSkillIcon(float time, int x, int y, const ZombieSkillHudIcon &icon) const
{
	if (icon.m_iCurrentSkill == ZOMBIE_SKILL_EMPTY)
		return x;

	int r, g, b;
	DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);

	if (m_iCurrentClass != ZOMBIE_CLASS_HUMAN)
	{
		// draws G->
		int w = gHUD.GetSpriteRect(m_HUD_zombieGKey).right - gHUD.GetSpriteRect(m_HUD_zombieGKey).left;
		// visible when ready
		if (icon.m_iCurrentSkillStatus == SKILL_STATUS_READY)
		{
			SPR_Set(gHUD.GetSprite(m_HUD_zombieGKey), 255, 193, 147);
			SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_zombieGKey));
		}
		x += w;
	}

	int iHudIcon = m_HUD_SkillIcons[icon.m_iCurrentSkill];
	int w = gHUD.GetSpriteRect(iHudIcon).right - gHUD.GetSpriteRect(iHudIcon).left;

	if (icon.m_iCurrentSkillStatus == SKILL_STATUS_USED)
		return x + w;

	if (time < icon.m_flTimeSkillReady)
	{
		wrect_t Rect = gHUD.GetSpriteRect(iHudIcon);
		int iHeight = Rect.bottom - Rect.top;

		float flPercent = (time - icon.m_flTimeSkillStart) / (icon.m_flTimeSkillReady - icon.m_flTimeSkillStart);


		DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);

		wrect_t BottomHalf = Rect;
		float deltaY = iHeight * (1.0f - flPercent);
		BottomHalf.top += deltaY;

		SPR_Set(gHUD.GetSprite(iHudIcon), r, g, b);
		SPR_DrawAdditive(0, x, y + deltaY, &BottomHalf);

		int a = 50;
		DrawUtils::ScaleColors(r, g, b, a);

		wrect_t TopHalf = Rect;
		TopHalf.bottom -= iHeight *  flPercent;

		SPR_Set(gHUD.GetSprite(iHudIcon), r, g, b);
		SPR_DrawAdditive(0, x, y, &TopHalf);
	}
	else
	{
		SPR_Set(gHUD.GetSprite(iHudIcon), 255, 193, 147);
		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(iHudIcon));
	}

	x += w;

	return x;
}

void CHudZB2_Skill::DrawSkillTip(float time) const
{
	int x = ScreenWidth / 2;
	int y = ScreenHeight * 2 / 3;
	for (auto &icon : m_ZombieSkillHudIcons)
	{
		if (icon.m_iCurrentSkill == ZOMBIE_SKILL_EMPTY)
			continue;
		if (time > icon.m_flTimeSkillBlink)
			continue;
		if (icon.m_iCurrentSkill > ZOMBIE_SKILL_CRAZY2)
			continue;
		auto &tex = m_pTexture_SkillTips[icon.m_iCurrentSkill];
		if (!tex)
			continue;

		int w = tex->w();
		int h = tex->h();

		float timeDelta = icon.m_flTimeSkillBlink - time;
		float modDelta = timeDelta - static_cast<float>(static_cast<int>(timeDelta));
		float a = modDelta < 0.5f ? modDelta * 2.0f : 2.0f - modDelta * 2.0f;

		tex->Draw2DQuadScaled(x - w / 2, y, x + w / 2, y + h, 0, 0, 1, 1, 255, 255, 255, 255 * a);

		y += h;
	}
}

void CHudZB2_Skill::DrawSkillBoardNew(float time) const
{
	int x = ScreenWidth / 2;
	int y = 58;

	if(gHUD.m_hudstyle->value == 2)
		y = 90;

	x -= m_pTexture_skillslotbg->w() / 2;
	if (m_iCurrentClass >= 0 && m_iCurrentClass < MAX_ZOMBIE_CLASS)
	{
		const auto &classicon = m_pTexture_NewClassIcons[m_iCurrentClass];
		if (classicon != nullptr)
		{
			x -= (classicon->w() + 4) / 2;

			classicon->Draw2DQuadScaled(x, y, x + classicon->w(), y + classicon->h());

			x += classicon->w() + 4;
		}
	}

	m_pTexture_skillslotbg->Draw2DQuadScaled(x, y, x + m_pTexture_skillslotbg->w(), y + m_pTexture_skillslotbg->h());

	char SkillKey = 'G';
	if (m_iCurrentClass == ZOMBIE_CLASS_HUMAN)
		SkillKey = '5';

	for (auto &icon : m_ZombieSkillHudIcons)
	{
		const auto &skillicon = m_pTexture_NewSkillIcons[icon.m_iCurrentSkill];
        const int w = m_pTexture_skillslotbg->w() / 4;
        const int h = m_pTexture_skillslotbg->h();
		if (skillicon && icon.m_iCurrentSkill >= 0 && icon.m_iCurrentSkill < MAX_ZOMBIE_SKILL)
		{
			if (time < icon.m_flTimeSkillReady)
			{
				// waiting for freezing
				float flPercent = (time - icon.m_flTimeSkillStart) / (icon.m_flTimeSkillReady - icon.m_flTimeSkillStart);

				float center_y = y + h * (1.0f - flPercent);

				// top half
				skillicon->Draw2DQuadScaled(x, y, x+w, center_y, 0, 0, 1, (1.0f - flPercent), 255, 255, 191, 50);

				// bottom half
				skillicon->Draw2DQuadScaled(x, center_y, x+w, (y + h), 0, (1.0f - flPercent), 1, 1, 230, 150, 150, 255);
			}
			else if (icon.m_iCurrentSkillStatus == SKILL_STATUS_USED)
			{
				// used
				skillicon->Draw2DQuadScaled(x, y, x + w, y + h, 0, 0, 1, 1, 255, 255, 191, 50);
			}
			else if (time < icon.m_flTimeSkillBlink)
			{
				// blinking
				float timeDelta = icon.m_flTimeSkillBlink - time;
				float modDelta = timeDelta - static_cast<float>(static_cast<int>(timeDelta));
				float a = modDelta < 0.5f ? modDelta * 2.0f : 2.0f - modDelta * 2.0f;
				a = 1.0 - a; // reverse ?

				skillicon->Draw2DQuadScaled(x, y, x + w, y + h, 0, 0, 1, 1, 255, 255, 191, 255 * a);
			}
			else
			{
				// normal
				skillicon->Draw2DQuadScaled(x, y, x + w, y + h, 0, 0, 1, 1, 255, 255, 191, 255);
			}
		}

		m_pTexture_skillslotkeybg->Draw2DQuadScaled(x - 3, y - 3, x + m_pTexture_skillslotkeybg->w(), y + m_pTexture_skillslotkeybg->h());

		DrawUtils::TextMessageDrawChar(x + 7, y, SkillKey, 200, 200, 200);

		SkillKey = SkillKey == 'G' ? '5' : SkillKey + 1;
		x += w;
	}
}

void CHudZB2_Skill::OnSkillInit(ZombieClassType zclass, ZombieSkillType skill1, ZombieSkillType skill2, ZombieSkillType skill3, ZombieSkillType skill4)
{
	assert(zclass >= 0 && zclass < MAX_ZOMBIE_CLASS);
	float flSkillBlinkTime = gHUD.m_flTime + 3.0f;
	m_iCurrentClass = zclass;
	if (zclass == ZOMBIE_CLASS_BOOSTER)
	{
		m_ZombieSkillHudIcons[0] = { skill1, SKILL_STATUS_USED, 0.0f, 0.0f, 0.0f };
		m_ZombieSkillHudIcons[1] = { skill2, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
		m_ZombieSkillHudIcons[2] = { skill3, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
		m_ZombieSkillHudIcons[3] = { skill4, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
	}
	else
	{
		m_ZombieSkillHudIcons[0] = { skill1, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
		m_ZombieSkillHudIcons[1] = { skill2, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
		m_ZombieSkillHudIcons[2] = { skill3, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
		m_ZombieSkillHudIcons[3] = { skill4, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
	}
	// some blink ?
	if (!m_pTexture_skillslotbg || !m_pTexture_skillslotbg->valid())
		return;

	int x = ScreenWidth / 2 - m_pTexture_skillslotbg->w() / 2;
	const int y = 58;
	if (m_iCurrentClass >= 0 && m_iCurrentClass < MAX_ZOMBIE_CLASS)
	{
		const auto& classicon = m_pTexture_NewClassIcons[m_iCurrentClass];
		if (classicon != nullptr)
		{
			x += (classicon->w() + 4) / 2;
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		auto& icon = m_ZombieSkillHudIcons[i];
		const auto& skillicon = m_pTexture_NewSkillIcons[icon.m_iCurrentSkill];

		char buf[256];
		sprintf(buf, "touch_removebutton \"_moe_skill%d_button\"\n", i);
		gEngfuncs.pfnClientCmd(buf);

        const int w = m_pTexture_skillslotbg->w() / 4;
        const int h = m_pTexture_skillslotbg->h();
		if (skillicon && icon.m_iCurrentSkill >= 0 && icon.m_iCurrentSkill < MAX_ZOMBIE_SKILL)
		{
			const float x1 = x / (float)ScreenWidth;
			const float y1 = y / (float)ScreenHeight;
			const float x2 = (x + w) / (float)ScreenWidth;
			const float y2 = (y + h) / (float)ScreenHeight;

			sprintf(buf, "alias +_moe_skill%d_press \"touch_setcolor _moe_skill%d_button 156 77 20 180\"\n", i, i);
			gEngfuncs.pfnClientCmd(buf);

			if(m_iCurrentClass == ZOMBIE_CLASS_HUMAN)
				sprintf(buf, "alias -_moe_skill%d_press \"MoE_HumanSkill%d; touch_setcolor _moe_skill%d_button 0 0 0 50\"\n", i, i + 1, i);
			else
				sprintf(buf, "alias -_moe_skill%d_press \"BTE_ZombieSkill%d; touch_setcolor _moe_skill%d_button 0 0 0 50\"\n", i, i + 1, i);

			gEngfuncs.pfnClientCmd(buf);
			sprintf(buf, "touch_addbutton \"_moe_skill%d_button\" \"*white\" \"+_moe_skill%d_press\" %f %f %f %f 0 0 0 50 260", i, i, x1, y1, x2, y2);
			gEngfuncs.pfnClientCmd(buf);
			//gMobileAPI.pfnTouchAddClientButton("_moe_skill1_button", "*white", "+_moe_skill1_press", 0.100000, 0.408511, 0.340000, 0.459574, color, 0, 0.0f, 0);

		}

		x += w;
	}

}

void CHudZB2_Skill::OnSkillActivate(ZombieSkillType skill, float flHoldTime, float flFreezeTime)
{
	for (int i = 0; i < 4; ++i)
	{
		auto& icon = m_ZombieSkillHudIcons[i];
		if (icon.m_iCurrentSkill == skill)
		{
			char buf[128];
			sprintf(buf, "touch_hide \"_moe_skill%d_button\"\n", i);
			gEngfuncs.pfnClientCmd(buf);
			icon = { skill, flFreezeTime > 0.0f ? SKILL_STATUS_FREEZING : SKILL_STATUS_USED, gHUD.m_flTime, gHUD.m_flTime + flFreezeTime, 0.0f };
		}
	}
}

void CHudZB2_Skill::OnSkillIconChange(ZombieSkillType skill1, ZombieSkillType skill2, ZombieSkillType skill3, ZombieSkillType skill4)
{
	ZombieSkillType Skill[4] = { skill1, skill2, skill3, skill4 };
	for (int i = 0; i < 4; ++i)
	{
		if (Skill[i] != ZOMBIE_SKILL_EMPTY)
			m_ZombieSkillHudIcons[i].m_iCurrentSkill = Skill[i];
		auto& icon = m_ZombieSkillHudIcons[i];
		if (icon.m_iCurrentSkill == Skill[i])
			icon = { Skill[i], SKILL_STATUS_READY, 0.0f, 0.0f, 0.0f };
	}
}

void CHudZB2_Skill::OnSkillIconPass(ZombieSkillType skill, ZombieSkillStatus status)
{
	auto& icon = m_ZombieSkillHudIcons[0];
	icon = { skill, status, 0.0f, 0.0f, 0.0f };
}

}
