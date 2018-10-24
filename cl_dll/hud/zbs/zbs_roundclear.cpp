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
#include <tuple>

const float ZBS_ROUNDCLEAR_DISPLAY_TIME = 1.0f;

int CHudZBSRoundClear::Init(void)
{
	return 1;
}

int CHudZBSRoundClear::VidInit(void)
{
	if(!m_pTexture_RoundClear)
		m_pTexture_RoundClear = R_LoadTextureShared("resource/hud/zbs/roundclear", TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	if (!m_pTexture_RoundFail)
		m_pTexture_RoundFail = R_LoadTextureShared("resource/hud/zbs/roundfail", TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	return 1;
}

void CHudZBSRoundClear::Shutdown(void)
{
	std::tie(m_pTexture_RoundClear, m_pTexture_RoundFail, m_pCurTexture) = std::make_tuple(nullptr,nullptr,nullptr);
}

int CHudZBSRoundClear::Draw(float time)
{
	if (!m_pCurTexture)
		return 1;

	if (time > m_flDisplayTime + 5.0f)
	{
		m_pCurTexture = nullptr;
		return 1;
	}

	int x = ScreenWidth / 2;
	int y = ScreenHeight / 4;
	const float flScale = 0.0f;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255 * std::min(5.0f - (time - m_flDisplayTime), 1.0f));
	m_pCurTexture->Bind();
	DrawUtils::Draw2DQuadScaled(x - 373 / 2, y, x + 373 / 2, y + 51);
	return 1;
}

void CHudZBSRoundClear::OnRoundClear()
{
	m_pCurTexture = m_pTexture_RoundClear;
	m_flDisplayTime = gHUD.m_flTime;
	gEngfuncs.pfnClientCmd("speak \"roundclear\"\n");
}

void CHudZBSRoundClear::OnRoundFail()
{
	m_pCurTexture = m_pTexture_RoundFail;
	m_flDisplayTime = gHUD.m_flTime;
	gEngfuncs.pfnClientCmd("speak \"roundfail\"\n");
}