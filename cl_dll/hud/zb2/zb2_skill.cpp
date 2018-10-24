#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zb2.h"
#include "zb2_skill.h"

#include <algorithm>
#include <functional>

const char *CHudZB2_Skill::ZOMBIE_SKILL_HUD_ICON[MAX_ZOMBIE_SKILL] = 
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

const char *CHudZB2_Skill::ZOMBIE_SKILL_HUD_TIP[MAX_ZOMBIE_SKILL] =
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

const char *CHudZB2_Skill::ZOMBIE_CLASS_HUD_ICON[MAX_ZOMBIE_CLASS] =
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

const char *CHudZB2_Skill::ZOMBIE_ITEM_HUD_ICON[2][3] = 
{
	{ "zombiACT", "zombiBCT", "zombiHCT" },
	{ "zombiATER", "zombiBTER", "zombiHTER" }
};

int CHudZB2_Skill::Init(void)
{
	m_HUD_zombirecovery = m_HUD_zombieGKey = 0;
	std::fill(std::begin(m_HUD_SkillIcons), std::end(m_HUD_SkillIcons), 0);
	std::fill(std::begin(m_HUD_ClassIcons), std::end(m_HUD_ClassIcons), 0);
	std::fill(std::begin(m_pTexture_SkillTips), std::end(m_pTexture_SkillTips), nullptr);
	return 1;
}

int CHudZB2_Skill::VidInit(void)
{
	m_HUD_zombirecovery = gHUD.GetSpriteIndex("zombirecovery");
	m_HUD_zombieGKey = gHUD.GetSpriteIndex("zombiGkey");
	for (int i = 0; i < MAX_ZOMBIE_SKILL; ++i)
	{
		if(ZOMBIE_SKILL_HUD_ICON[i][0] != '\0')
			m_HUD_SkillIcons[i] = gHUD.GetSpriteIndex(ZOMBIE_SKILL_HUD_ICON[i]);
	}

	for (int i = 0; i < MAX_ZOMBIE_CLASS; ++i)
	{
		if (ZOMBIE_CLASS_HUD_ICON[i][0] != '\0')
			m_HUD_ClassIcons[i] = gHUD.GetSpriteIndex(ZOMBIE_CLASS_HUD_ICON[i]);
	}

	for (int i = 0; i < MAX_ZOMBIE_SKILL; ++i)
	{
		if (ZOMBIE_SKILL_HUD_TIP[i][0] != '\0')
			m_pTexture_SkillTips[i] = R_LoadTextureUnique(ZOMBIE_SKILL_HUD_TIP[i]);
	}

	return 1;
}

void CHudZB2_Skill::Shutdown(void)
{
	std::fill(std::begin(m_pTexture_SkillTips), std::end(m_pTexture_SkillTips), nullptr);
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
	x = DrawSkillBoard(time, x, y);

	DrawSkillTip(time);

	return 1;
}

void CHudZB2_Skill::Think()
{
	for (auto &icon : m_ZombieSkillHudIcons)
	{
		if (icon.m_iCurrentSkillStatus == SKILL_STATUS_USING || icon.m_iCurrentSkillStatus == SKILL_STATUS_FREEZING)
		{
			if (gHUD.m_flTime > icon.m_flTimeSkillReady)
			{
				icon.m_iCurrentSkillStatus = SKILL_STATUS_READY;
				icon.m_flTimeSkillBlink = gHUD.m_flTime + 3.0f;
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
		if (!tex || !tex->IsValid())
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

void CHudZB2_Skill::OnSkillInit(ZombieClassType zclass, ZombieSkillType skill1, ZombieSkillType skill2, ZombieSkillType skill3, ZombieSkillType skill4)
{
	float flSkillBlinkTime = gHUD.m_flTime + 3.0f;
	m_iCurrentClass = zclass;
	m_ZombieSkillHudIcons[0] = { skill1, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
	m_ZombieSkillHudIcons[1] = { skill2, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
	m_ZombieSkillHudIcons[2] = { skill3, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
	m_ZombieSkillHudIcons[3] = { skill4, SKILL_STATUS_READY, 0.0f, 0.0f, flSkillBlinkTime };
	// some blink ? 
}

void CHudZB2_Skill::OnSkillActivate(ZombieSkillType skill, float flHoldTime, float flFreezeTime)
{
	for (auto &icon : m_ZombieSkillHudIcons)
	{
		if (icon.m_iCurrentSkill == skill)
		{
			icon = { skill, flFreezeTime > 0.0f ? SKILL_STATUS_FREEZING : SKILL_STATUS_USED, gHUD.m_flTime, gHUD.m_flTime + flFreezeTime, 0.0f };
		}
	}
}