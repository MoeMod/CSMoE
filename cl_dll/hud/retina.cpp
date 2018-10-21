#include "hud.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "retina.h"

#include <algorithm> // std::remove_if

int CHudRetina::Init(void)
{
	

	gHUD.AddHudElem(this);
	m_iFlags = 0;
	return 1;
}

int CHudRetina::VidInit(void)
{
	
	m_iFlags |= HUD_ACTIVE;
	return 1;
}

void CHudRetina::Shutdown(void)
{
	for (auto &kv : m_TextureMap)
	{
		gRenderAPI.GL_FreeTexture(kv.second);
	}
	m_TextureMap.clear();
}

void CHudRetina::Reset(void)
{
	//m_ItemList.clear();
}

void CHudRetina::Think(void)
{
	// remove all expired item...
	m_ItemList.erase(std::remove_if(m_ItemList.begin(), m_ItemList.end(), [](const RetinaDrawItem_s &item) {return item.flTimeEnd > 0.0f && gHUD.m_flTime > item.flTimeEnd; }), m_ItemList.end());
}

int CHudRetina::Draw(float time)
{
	for (auto &item : m_ItemList)
		DrawItem(time, item);
	return 1;
}

void CHudRetina::DrawItem(float time, const RetinaDrawItem_s &item) const
{
	float a = 1.0f;
	if (item.type & RETINA_DRAW_TYPE_BLINK)
	{
		float timeDelta = item.flTimeEnd - time;
		float modDelta = timeDelta - static_cast<float>(static_cast<int>(timeDelta));
		a = 0.5f + (modDelta < 0.5f ? modDelta * 2.0f : 2.0f - modDelta * 2.0f) * 0.5f;
	}

	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255 * a);
	gRenderAPI.GL_SelectTexture(0);
	gRenderAPI.GL_Bind(0, item.iTexture);

	if (item.type & RETINA_DRAW_TYPE_QUARTER)
	{
		DrawUtils::Draw2DQuad(0,			0,				TrueWidth / 2,	TrueHeight / 2); // ¨I	
		DrawUtils::Draw2DQuad(TrueWidth,	0,				TrueWidth / 2,	TrueHeight / 2); // ¨J
		DrawUtils::Draw2DQuad(0,			TrueHeight, 	TrueWidth / 2,	TrueHeight / 2); // ¨L
		DrawUtils::Draw2DQuad(TrueWidth,	TrueHeight,		TrueWidth / 2,	TrueHeight / 2); // ¨K
	}
	else
	{
		DrawUtils::Draw2DQuad(0, 0, TrueWidth, TrueHeight);
	}
}

void CHudRetina::RemoveAll()
{
	m_ItemList.clear();
}

auto CHudRetina::AddItem(const char *path, RetinaDrawType_e type, float time) -> RetinaItemIndex_t
{
	int tex = PrecacheTexture(path);
	float flTimeEnd = time <= 0.0f ? time : gHUD.m_flTime + time;
	m_ItemList.push_back({ type, tex, flTimeEnd });
	return m_ItemList.size() - 1;
}

auto CHudRetina::RemoveItem(RetinaItemIndex_t idx) -> RetinaDrawItem_s
{
	assert(idx >= 0 && idx < m_ItemList.size());

	RetinaDrawItem_s item = m_ItemList[idx];
	m_ItemList.erase(m_ItemList.begin() + idx);
	return item;
}

int CHudRetina::PrecacheTexture(const char *path)
{
	auto iter = m_TextureMap.find(path);
	if (iter == m_TextureMap.end())
	{
		int tex = gRenderAPI.GL_LoadTexture(path, NULL, 0, TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP);
		iter = m_TextureMap.emplace(path, tex).first;
	}
	return iter->second;
}