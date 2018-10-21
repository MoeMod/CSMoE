#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zbs.h"
#include "zbs_scoreboard.h"

#include "string.h"
#include "assert.h"

inline int CalcTeamFrags()
{
	int result = 0;
	for (auto &info : g_PlayerExtraInfo)
		result += info.frags;
	return result;
}

inline void BuildNumberRC(wrect_t(&rgrc)[10], int w, int h)
{
	// legacy code from CSBTE Csoldjb 10.1
	int nw = 0;

	for (int i = 0; i < 10; i++)
	{
		rgrc[i].left = nw;
		rgrc[i].top = 0;
		rgrc[i].right = rgrc[i].left + w;
		rgrc[i].bottom = h;

		nw += w;
	}
}

inline void BuildNumberRC(wrect_t(&rgrc)[10], int tex)
{
	int w = gRenderAPI.RenderGetParm(PARM_TEX_SRC_WIDTH, tex);
	int h = gRenderAPI.RenderGetParm(PARM_TEX_SRC_HEIGHT, tex);
	return BuildNumberRC(rgrc, w / 10, h);
}

inline void DrawTexturePart(int tex, const wrect_t &rect, int x1, int y1, float scale = 1.0f)
{
	gRenderAPI.GL_Bind(0, tex);

	float w = static_cast<float>(gRenderAPI.RenderGetParm(PARM_TEX_SRC_WIDTH, tex));
	float h = static_cast<float>(gRenderAPI.RenderGetParm(PARM_TEX_SRC_HEIGHT, tex));

	x1 *= gHUD.m_flScale;
	y1 *= gHUD.m_flScale;
	scale *= gHUD.m_flScale;

	int x2 = x1 + (rect.right - rect.left) * scale;
	int y2 = y1 + (rect.bottom - rect.top) * scale;

	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	gEngfuncs.pTriAPI->TexCoord2f(rect.left / w, rect.top / h);
	gEngfuncs.pTriAPI->Vertex3f(x1, y1, 0);
	gEngfuncs.pTriAPI->TexCoord2f(rect.left / w, rect.bottom / h);
	gEngfuncs.pTriAPI->Vertex3f(x1, y2, 0);
	gEngfuncs.pTriAPI->TexCoord2f(rect.right / w, rect.bottom / h);
	gEngfuncs.pTriAPI->Vertex3f(x2, y2, 0);
	gEngfuncs.pTriAPI->TexCoord2f(rect.right / w, rect.top / h);
	gEngfuncs.pTriAPI->Vertex3f(x2, y1, 0);
	gEngfuncs.pTriAPI->End();
}

inline int DrawTexturedNumbersTopRightAligned(int tex, const wrect_t(&rect)[10], int iNumber, int x, int y, float scale = 1.0f)
{
	assert(iNumber >= 0);

	do
	{
		int k = iNumber % 10;
		iNumber /= 10;
		DrawTexturePart(tex, rect[k], x, y, scale);
		x -= (rect[k].right - rect[k].left) * scale;
	} while (iNumber > 0);

	return x;
}

inline unsigned math_log10(unsigned v)
{
	return (v >= 1000000000) ? 9 : (v >= 100000000) ? 8 : (v >= 10000000) ? 7 :
		(v >= 1000000) ? 6 : (v >= 100000) ? 5 : (v >= 10000) ? 4 :
		(v >= 1000) ? 3 : (v >= 100) ? 2 : (v >= 10) ? 1 : 0;
}

inline int DrawTexturedNumbersTopCenterAligned(int tex, const wrect_t(&rect)[10], int iNumber, int x, int y, float scale = 1.0f)
{
	int n = math_log10(iNumber);
	x += (rect[0].right - rect[0].left) * (n - 1) * scale * 0.5f;
	return DrawTexturedNumbersTopRightAligned(tex, rect, iNumber, x, y, scale);
}

int CHudZBSScoreBoard::Init(void)
{
	m_iBackground = m_iTeamnumber = m_iSelfnumber = m_iToprecord = 0;

	return 1;
}

int CHudZBSScoreBoard::VidInit(void)
{
	m_iBackground = gRenderAPI.GL_LoadTexture("resource/hud/zbs/zbsboard", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);

	m_iTeamnumber = gRenderAPI.GL_LoadTexture("resource/hud/zbs/teamnumber", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	m_iSelfnumber = gRenderAPI.GL_LoadTexture("resource/hud/zbs/selfnumber", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
	m_iToprecord = gRenderAPI.GL_LoadTexture("resource/hud/challenge/toprecord", NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);

	BuildNumberRC(m_rcTeamnumber, 23, 24);
	BuildNumberRC(m_rcSelfnumber, 19, 19);
	BuildNumberRC(m_rcToprecord, 14, 14);

	return 1;
}

void CHudZBSScoreBoard::Shutdown(void)
{
	gRenderAPI.GL_FreeTexture(m_iBackground);
	gRenderAPI.GL_FreeTexture(m_iTeamnumber);
	gRenderAPI.GL_FreeTexture(m_iSelfnumber);
	gRenderAPI.GL_FreeTexture(m_iToprecord);
}

int CHudZBSScoreBoard::Draw(float time)
{
	int x = ScreenWidth / 2;
	int y = 5;
	const float flScale = 0.0f;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
	gRenderAPI.GL_SelectTexture(0);
	gRenderAPI.GL_Bind(0, m_iBackground);
	DrawUtils::Draw2DQuadScaled(x - 373 / 2, y, x + 373 / 2, y + 51);
	
	int idx = gEngfuncs.GetLocalPlayer()->index;
	int roundNumber = gHUD.m_Scoreboard.m_iTeamScore_CT + 1;
	int selfKill = g_PlayerExtraInfo[idx].frags;
	int teamKill = CalcTeamFrags();

	DrawTexturedNumbersTopCenterAligned(m_iToprecord, m_rcToprecord, roundNumber, x + 25, y + 25); // ok
	DrawTexturedNumbersTopCenterAligned(m_iTeamnumber, m_rcTeamnumber, teamKill, x - 80, y + 10);

	gEngfuncs.pTriAPI->Color4ub(160, 210, 250, 255);
	DrawTexturedNumbersTopCenterAligned(m_iSelfnumber, m_rcSelfnumber, selfKill, x + 105, y + 10);
	
	return 1;
}