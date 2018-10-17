#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zb2.h"
#include "zb2_skill.h"

#include <algorithm>
#include <functional>

int CHudZB2_Skill::Init(void)
{
	m_HUD_zombirecovery = 0;
	return 1;
}

int CHudZB2_Skill::VidInit(void)
{
	m_HUD_zombirecovery = gHUD.GetSpriteIndex("zombirecovery");
	return 1;
}

void CHudZB2_Skill::Shutdown(void)
{
	
}

void CHudZB2_Skill::Reset(void)
{
	
}

int CHudZB2_Skill::Draw(float time)
{
	DrawHealthRecoveryIcon(time);
	return 1;
}

void CHudZB2_Skill::Think()
{
	
}

void CHudZB2_Skill::OnHealthRecovery()
{
	// begin draw HUD
	m_flRecoveryBeginTime = gHUD.m_flTime;
}

void CHudZB2_Skill::DrawHealthRecoveryIcon(float time)
{
	if (time > m_flRecoveryBeginTime + 1.0f)
		return;

	int a = 255.0f - (time - m_flRecoveryBeginTime) * 255.0f;

	int r, g, b;
	DrawUtils::UnpackRGB(r, g, b, RGB_WHITE);
	DrawUtils::ScaleColors(r, g, b, a);

	int iWidth = gHUD.GetSpriteRect(m_HUD_zombirecovery).right - gHUD.GetSpriteRect(m_HUD_zombirecovery).left;
	int iHeight = gHUD.GetSpriteRect(m_HUD_zombirecovery).bottom - gHUD.GetSpriteRect(m_HUD_zombirecovery).top;
	int x = 0; // iWidth / 2;
	int y = ScreenHeight - gHUD.m_iFontHeight * 3 / 2 - iHeight;

	SPR_Set(gHUD.GetSprite(m_HUD_zombirecovery), r, g, b);
	SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_zombirecovery));
}