#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zbs.h"
#include "zbs_roundclear.h"

#include "string.h"
#include "assert.h"
#include <numeric>

const float ZBS_ROUNDCLEAR_DISPLAY_TIME = 1.0f;

int CHudZBSRoundClear::Init(void)
{
	m_iRoundClear = m_iRoundFail = 0;
	m_iCurTexture = 0;
	return 1;
}

int CHudZBSRoundClear::VidInit(void)
{
	m_iRoundClear = gRenderAPI.GL_LoadTexture("resource/hud/zbs/roundclear", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	m_iRoundFail = gRenderAPI.GL_LoadTexture("resource/hud/zbs/roundfail", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	m_iCurTexture = 0;
	return 1;
}

void CHudZBSRoundClear::Shutdown(void)
{
	gRenderAPI.GL_FreeTexture(m_iRoundClear);
	gRenderAPI.GL_FreeTexture(m_iRoundFail);
	m_iCurTexture = 0;
}

int CHudZBSRoundClear::Draw(float time)
{
	if (!m_iCurTexture)
		return 1;

	if (time > m_flDisplayTime + 5.0f)
	{
		m_iCurTexture = 0;
		return 1;
	}

	int x = ScreenWidth / 2;
	int y = ScreenHeight / 4;
	const float flScale = 0.0f;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255 * std::min(5.0f - (time - m_flDisplayTime), 1.0f));
	gRenderAPI.GL_SelectTexture(0);
	gRenderAPI.GL_Bind(0, m_iRoundClear);
	DrawUtils::Draw2DQuadScaled(x - 373 / 2, y, x + 373 / 2, y + 51);
	return 1;
}

void CHudZBSRoundClear::OnRoundClear()
{
	m_iCurTexture = m_iRoundClear;
	m_flDisplayTime = gHUD.m_flTime;
	gEngfuncs.pfnClientCmd("speak \"roundclear\"\n");
}

void CHudZBSRoundClear::OnRoundFail()
{
	m_iCurTexture = m_iRoundFail;
	m_flDisplayTime = gHUD.m_flTime;
	gEngfuncs.pfnClientCmd("speak \"roundfail\"\n");
}