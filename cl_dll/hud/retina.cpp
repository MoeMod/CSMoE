/*
retina.cpp - CSMoE Client HUD : Retina
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
#include "cl_util.h"

#include "retina.h"

#include <algorithm> // std::remove_if

namespace cl {

int CHudRetina::Init(void)
{


	gHUD.AddHudElem(this);
	m_iFlags = 0;
	return 1;
}

int CHudRetina::VidInit(void)
{
	RemoveAll();
	m_iFlags |= HUD_ACTIVE;
	return 1;
}

void CHudRetina::Shutdown(void)
{
	RemoveAll();
}

void CHudRetina::Reset(void)
{
	//RemoveAll();
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
	if (!item.pTexture)
		return;

	float a = 1.0f;
	byte r = 255;
	byte g = 255;
	byte b = 255;
	if (item.type & RETINA_DRAW_TYPE_BLINK)
	{
		float timeDelta = item.flTimeEnd - time;
		float modDelta = timeDelta - static_cast<float>(static_cast<int>(timeDelta));
		a = 0.5f + (modDelta < 0.5f ? modDelta * 2.0f : 2.0f - modDelta * 2.0f) * 0.5f;
	}

	if (item.type & RETINA_DRAW_TYPE_BLACK)
	{
		r = 0;
		g = 0;
		b = 0;
	}

	if (item.type & RETINA_DRAW_TYPE_QUARTER)
	{
		item.pTexture->Draw2DQuad(0,			0,				TrueWidth / 2,	TrueHeight / 2, 0, 0, 1, 1, r, g, b, 255 * a); // �I
		item.pTexture->Draw2DQuad(TrueWidth,	0,				TrueWidth / 2,	TrueHeight / 2, 0, 0, 1, 1, r, g, b, 255 * a); // �J
		item.pTexture->Draw2DQuad(0,			TrueHeight, 	TrueWidth / 2,	TrueHeight / 2, 0, 0, 1, 1, r, g, b, 255 * a); // �L
		item.pTexture->Draw2DQuad(TrueWidth,	TrueHeight,		TrueWidth / 2,	TrueHeight / 2, 0, 0, 1, 1, r, g, b, 255 * a); // �K
	}
	else
	{
		item.pTexture->Draw2DQuad(0, 0, TrueWidth, TrueHeight, 0, 0, 1, 1, r, g, b, 255 * a);
	}
}

void CHudRetina::RemoveAll()
{
	m_ItemList.clear();
}

auto CHudRetina::AddItem(SharedTexture tex, int type, float time, MagicNumber num) -> MagicNumber
{
	float flTimeEnd = time <= 0.0f ? time : gHUD.m_flTime + time;
	if(time > 0)
		m_ItemList.push_back({ type, tex, flTimeEnd, num });
	return num;
}

auto CHudRetina::AddItem(SharedTexture tex, int type, float time) -> MagicNumber
{
	return AddItem(tex, type, time, std::hash<SharedTexture>()(tex));
}

bool CHudRetina::RemoveItem(MagicNumber idx)
{
	auto iter_new_end = std::remove_if(m_ItemList.begin(), m_ItemList.end(), [idx](const RetinaDrawItem_s &item) {return item.num == idx; });
	bool success = iter_new_end != m_ItemList.end();
	m_ItemList.erase(iter_new_end, m_ItemList.end());
	return success;
}

SharedTexture CHudRetina::PrecacheTexture(const char *path)
{
	auto iter = m_TextureMap.find(path);
	if (iter == m_TextureMap.end())
	{
		iter = m_TextureMap.emplace(path, R_LoadTextureShared(path)).first;
	}
	return iter->second;
}

}