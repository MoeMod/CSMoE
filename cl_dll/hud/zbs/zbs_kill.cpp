#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zbs.h"
#include "zbs_kill.h"

#include <algorithm>
#include <functional>

const float ZBS_KILL_DISPLAY_TIME = 1.0f;

void CHudZBSKill::OnKillMessage()
{
	m_killTimes.push_back(gHUD.m_flTime);
}

int CHudZBSKill::Init(void)
{
	m_iKillTexture = 0;

	return 1;
}

int CHudZBSKill::VidInit(void)
{
	m_iKillTexture = gRenderAPI.GL_LoadTexture("resource/hud/zbs/zbskill", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	
	return 1;
}

void CHudZBSKill::Shutdown(void)
{
	gRenderAPI.GL_FreeTexture(m_iKillTexture);
}

void CHudZBSKill::Reset(void)
{
	m_killTimes.clear();
}

int CHudZBSKill::Draw(float time)
{
	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
	gRenderAPI.GL_SelectTexture(0);
	gRenderAPI.GL_Bind(0, m_iKillTexture);
	
	for (auto &flStartTime : m_killTimes)
	{
		float shownTime = time - flStartTime;
		if (shownTime > 1.0f)
			continue;

		int x = ScreenWidth / 2 - 234 / 2;
		int y = ScreenHeight / 4 - shownTime * 100.0f;

		
		float a = (flStartTime - gHUD.m_flTime) / ZBS_KILL_DISPLAY_TIME;
		gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255 * a);
		
		DrawUtils::Draw2DQuad(x, y, x + 234, y + 55);
	}

	return 1;
}

void CHudZBSKill::Think()
{
	// remove expired item
	m_killTimes.erase(std::remove_if(m_killTimes.begin(), m_killTimes.end(), std::bind(std::less<float>(), std::placeholders::_1, gHUD.m_flTime - ZBS_KILL_DISPLAY_TIME)), m_killTimes.end());
}