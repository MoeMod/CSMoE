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
#include "triangleapi.h"

#include "zb2.h"
#include "zb2_skill.h"

#include <algorithm>
#include <functional>

struct CHudZB2_Skill::Config
{
	static const char * const ZOMBIE_SKILL_HUD_ICON[MAX_ZOMBIE_SKILL];
	static const char * const ZOMBIE_SKILL_HUD_TIP[MAX_ZOMBIE_SKILL];
	static const char * const ZOMBIE_CLASS_HUD_ICON[MAX_ZOMBIE_CLASS];
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
		"zombicrazy2" // ZOMBIE_SKILL_CRAZY2,
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
		"resource/helperhud/crazyspeed" // ZOMBIE_SKILL_CRAZY2,
};

const char *  const CHudZB2_Skill::Config::ZOMBIE_CLASS_HUD_ICON[MAX_ZOMBIE_CLASS] =
{
		"", // ZOMBIE_CLASS_HUMAN
		"", // ZOMBIE_CLASS_TANK,
		"zombiDTER", // ZOMBIE_CLASS_SPEED,
		"zombiETER", // ZOMBIE_CLASS_HEAVY,
		"zombiFTER", // ZOMBIE_CLASS_PC,
		"zombiGTER", // ZOMBIE_CLASS_HEAL,
		"zombiITER", // ZOMBIE_CLASS_DEIMOS,
		"zombiJTER", // ZOMBIE_CLASS_DEIMOS2,
};

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
		"resource/zombi/zombieskill_zombicrazy2" // ZOMBIE_SKILL_CRAZY2,
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
	m_HUD_ClassIcons{},
	m_flRecoveryBeginTime(0.0f),
	m_iCurrentClass(ZOMBIE_CLASS_HUMAN),
	m_ZombieSkillHudIcons{}
{
	touch_enable = gEngfuncs.pfnGetCvarPointer("touch_enable");
}

int CHudZB2_Skill::VidInit(void)
{
	m_HUD_zombirecovery = gHUD.GetSpriteIndex("zombirecovery");
	m_HUD_zombieGKey = gHUD.GetSpriteIndex("zombiGkey");
	for (int i = 0; i < MAX_ZOMBIE_SKILL; ++i)
	{
		if(Config::ZOMBIE_SKILL_HUD_ICON[i][0] != '\0')
			m_HUD_SkillIcons[i] = gHUD.GetSpriteIndex(Config::ZOMBIE_SKILL_HUD_ICON[i]);
	}

	for (int i = 0; i < MAX_ZOMBIE_CLASS; ++i)
	{
		if (Config::ZOMBIE_CLASS_HUD_ICON[i][0] != '\0')
			m_HUD_ClassIcons[i] = gHUD.GetSpriteIndex(Config::ZOMBIE_CLASS_HUD_ICON[i]);
	}

	for (int i = 0; i < MAX_ZOMBIE_SKILL; ++i)
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
		if (Config::ZOMBIE_SKILL_HUD_ICON_NEW[i][0] != '\0')
			if (!m_pTexture_NewSkillIcons[i])
				m_pTexture_NewSkillIcons[i] = R_LoadTextureUnique(Config::ZOMBIE_SKILL_HUD_ICON_NEW[i]);
	}

	for (int i = 0; i < MAX_ZOMBIE_CLASS; ++i)
	{
		if (Config::ZOMBIE_CLASS_HUD_ICON_NEW[i][0] != '\0')
			if (!m_pTexture_NewClassIcons[i])
				m_pTexture_NewClassIcons[i] = R_LoadTextureUnique(Config::ZOMBIE_CLASS_HUD_ICON_NEW[i]);
	}

	return 1;
}

void CHudZB2_Skill::Reset(void)
{
	OnSkillInit();
}

int CHudZB2_Skill::Draw(float time)
{
	int iHeight = gHUD.GetSpriteRect(m_HUD_zombirecovery).bottom - gHUD.GetSpriteRect(m_HUD_zombirecovery).top;
	int x = 0; // iWidth / 2;
	int y = ScreenHeight - gHUD.m_iFontHeight * 3 / 2 - iHeight;

	x = DrawHealthRecoveryIcon(time, x, y);
	if (!touch_enable || !touch_enable->value)
	{
		x = DrawSkillBoard(time, x, y);
		DrawSkillTip(time);
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
			SPR_Set(gHUD.GetSprite(m_HUD_zombieGKey), r, g, b);
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
		DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);
		SPR_Set(gHUD.GetSprite(iHudIcon), r, g, b);
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
		auto &tex = m_pTexture_SkillTips[icon.m_iCurrentSkill];
		if (!tex)
			continue;

		int w = tex->w();
		int h = tex->h();

		float timeDelta = icon.m_flTimeSkillBlink - time;
		float modDelta = timeDelta - static_cast<float>(static_cast<int>(timeDelta));
		float a = modDelta < 0.5f ? modDelta * 2.0f : 2.0f - modDelta * 2.0f;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
		gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255 * a);
		tex->Bind();

		DrawUtils::Draw2DQuadScaled(x - w / 2, y, x + w / 2, y + h);

		y += h;
	}
}

void CHudZB2_Skill::DrawSkillBoardNew(float time) const
{
	int x = ScreenWidth / 2;
	int y = 58;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);

	x -= m_pTexture_skillslotbg->w() / 2;
	if (m_iCurrentClass >= 0 && m_iCurrentClass < MAX_ZOMBIE_CLASS)
	{
		const auto &classicon = m_pTexture_NewClassIcons[m_iCurrentClass];
		if (classicon != nullptr)
		{
			x -= (classicon->w() + 4) / 2;

			gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
			classicon->Bind();
			DrawUtils::Draw2DQuadScaled(x, y, x + classicon->w(), y + classicon->h());

			x += classicon->w() + 4;
		}
	}

	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
	m_pTexture_skillslotbg->Bind();
	DrawUtils::Draw2DQuadScaled(x, y, x + m_pTexture_skillslotbg->w(), y + m_pTexture_skillslotbg->h());

	char SkillKey = 'G';
	if (m_iCurrentClass == ZOMBIE_CLASS_HUMAN)
		SkillKey = '5';

	for (auto &icon : m_ZombieSkillHudIcons)
	{
		const auto &skillicon = m_pTexture_NewSkillIcons[icon.m_iCurrentSkill];
		if (skillicon && icon.m_iCurrentSkill >= 0 && icon.m_iCurrentSkill < MAX_ZOMBIE_SKILL)
		{
			if (time < icon.m_flTimeSkillReady)
			{
				// waiting for freezing
				float flPercent = (time - icon.m_flTimeSkillStart) / (icon.m_flTimeSkillReady - icon.m_flTimeSkillStart);

				float center_y = y + skillicon->h() * (1.0f - flPercent);

				// top half
				gEngfuncs.pTriAPI->Color4ub(255, 255, 191, 50);
				skillicon->Bind();

				gEngfuncs.pTriAPI->Begin(TRI_QUADS);
				// �I (x1, y1)
				gEngfuncs.pTriAPI->TexCoord2f(0, 0);
				gEngfuncs.pTriAPI->Vertex3f(x * gHUD.m_flScale, y * gHUD.m_flScale, 0);
				// �L (x1, y2)
				gEngfuncs.pTriAPI->TexCoord2f(0, (1.0f - flPercent));
				gEngfuncs.pTriAPI->Vertex3f(x * gHUD.m_flScale, center_y * gHUD.m_flScale, 0);
				// �K (x2, y2)
				gEngfuncs.pTriAPI->TexCoord2f(1, (1.0f - flPercent));
				gEngfuncs.pTriAPI->Vertex3f((x + skillicon->w()) * gHUD.m_flScale, center_y * gHUD.m_flScale, 0);
				// �J (x2, y1)
				gEngfuncs.pTriAPI->TexCoord2f(1, 0);
				gEngfuncs.pTriAPI->Vertex3f((x + skillicon->w()) * gHUD.m_flScale, y * gHUD.m_flScale, 0);
				gEngfuncs.pTriAPI->End();

				// bottom half
				gEngfuncs.pTriAPI->Color4ub(230, 150, 150, 255);
				skillicon->Bind();

				gEngfuncs.pTriAPI->Begin(TRI_QUADS);
				// �I (x1, y1)
				gEngfuncs.pTriAPI->TexCoord2f(0, (1.0f - flPercent));
				gEngfuncs.pTriAPI->Vertex3f(x * gHUD.m_flScale, center_y * gHUD.m_flScale, 0);
				// �L (x1, y2)
				gEngfuncs.pTriAPI->TexCoord2f(0, 1);
				gEngfuncs.pTriAPI->Vertex3f(x * gHUD.m_flScale, (y + skillicon->h()) * gHUD.m_flScale, 0);
				// �K (x2, y2)
				gEngfuncs.pTriAPI->TexCoord2f(1, 1);
				gEngfuncs.pTriAPI->Vertex3f((x + skillicon->w()) * gHUD.m_flScale, (y + skillicon->h()) * gHUD.m_flScale, 0);
				// �J (x2, y1)
				gEngfuncs.pTriAPI->TexCoord2f(1, (1.0f - flPercent));
				gEngfuncs.pTriAPI->Vertex3f((x + skillicon->w()) * gHUD.m_flScale, center_y * gHUD.m_flScale, 0);
				gEngfuncs.pTriAPI->End();
			}
			else if (icon.m_iCurrentSkillStatus == SKILL_STATUS_USED)
			{
				// used
				gEngfuncs.pTriAPI->Color4ub(255, 255, 191, 50);
				skillicon->Bind();
				DrawUtils::Draw2DQuadScaled(x, y, x + skillicon->w(), y + skillicon->h());
			}
			else if (time < icon.m_flTimeSkillBlink)
			{
				// blinking
				float timeDelta = icon.m_flTimeSkillBlink - time;
				float modDelta = timeDelta - static_cast<float>(static_cast<int>(timeDelta));
				float a = modDelta < 0.5f ? modDelta * 2.0f : 2.0f - modDelta * 2.0f;
				a = 1.0 - a; // reverse ?

				gEngfuncs.pTriAPI->Color4ub(255, 255, 191, 255 * a);
				skillicon->Bind();
				DrawUtils::Draw2DQuadScaled(x, y, x + skillicon->w(), y + skillicon->h());
			}
			else
			{
				// normal
				gEngfuncs.pTriAPI->Color4ub(255, 255, 191, 255);
				skillicon->Bind();
				DrawUtils::Draw2DQuadScaled(x, y, x + skillicon->w(), y + skillicon->h());
			}
		}
		
		gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
		gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
		m_pTexture_skillslotkeybg->Bind();
		DrawUtils::Draw2DQuadScaled(x - 3, y - 3, x + m_pTexture_skillslotkeybg->w(), y + m_pTexture_skillslotkeybg->h());

		DrawUtils::TextMessageDrawChar(x + 7, y, SkillKey, 100, 100, 100);

		SkillKey = SkillKey == 'G' ? '5' : SkillKey + 1;
		x += 58;
	}
}

void CHudZB2_Skill::OnSkillInit(ZombieClassType zclass, ZombieSkillType skill1, ZombieSkillType skill2, ZombieSkillType skill3, ZombieSkillType skill4)
{
	assert(zclass >= 0 && zclass < MAX_ZOMBIE_CLASS);
	float flSkillBlinkTime = gHUD.m_flTime + 3.0f;
	m_iCurrentClass = zclass;
	m_ZombieSkillHudIcons[0] = { skill1, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
	m_ZombieSkillHudIcons[1] = { skill2, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
	m_ZombieSkillHudIcons[2] = { skill3, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
	m_ZombieSkillHudIcons[3] = { skill4, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
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

		if (skillicon && icon.m_iCurrentSkill >= 0 && icon.m_iCurrentSkill < MAX_ZOMBIE_SKILL)
		{
			const float x1 = x / (float)ScreenWidth;
			const float y1 = y / (float)ScreenHeight;
			const float x2 = (x + skillicon->w()) / (float)ScreenWidth;
			const float y2 = (y + skillicon->h()) / (float)ScreenHeight;

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
		
		x += 58;
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