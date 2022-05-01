/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once
#include <vector>
#include <string>
#include <map>

#include "r_texture.h"
namespace cl {
class CHudRetina : public CHudBase
{
public:
	int Init(void) override;
	int VidInit(void) override;
	void Think(void) override;
	void Reset(void) override;
	int Draw(float time) override;
	void Shutdown(void) override;

public:
	using MagicNumber = size_t;
	enum RetinaDrawType_e : int
	{
		RETINA_DRAW_TYPE_NONE,
		RETINA_DRAW_TYPE_QUARTER = (1 << 0),
		RETINA_DRAW_TYPE_BLINK = (1 << 1),
		RETINA_DRAW_TYPE_BLACK = (1 << 2),
	};

	struct RetinaDrawItem_s
	{
		int type;
		SharedTexture pTexture;
		float flTimeEnd;
		MagicNumber num;
	};

	MagicNumber AddItem(SharedTexture tex, int type = RETINA_DRAW_TYPE_NONE, float time = -1.0f);
	MagicNumber AddItem(SharedTexture tex, int type, float time, MagicNumber num);
	bool RemoveItem(MagicNumber idx);
	void RemoveAll();
	void DrawItem(float time, const RetinaDrawItem_s &item) const;
	SharedTexture PrecacheTexture(const char *path);

private:
	std::vector<RetinaDrawItem_s> m_ItemList;
	std::map<std::string, SharedTexture> m_TextureMap;
};
}