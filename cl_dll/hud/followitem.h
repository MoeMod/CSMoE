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
#include "utlvector.h"
#include <unordered_map>

#include "r_texture.h"
namespace cl {
class CHudFollowItem : public CHudBase
{
public:
	int Init(void) override;
	int VidInit(void) override;
	void Reset(void) override;
	int Draw(float time) override;

public:
	void SetIconItem(int iType, const Vector& pos, bool bEnable, float flLife);
	void DrawRadarItems();

public:
	class supplyItem
	{
	public:
		int entity;
		Vector pos;

	public:
		inline int operator==(const supplyItem& src) const { return src.entity == this->entity; }
	};

	CUtlVector<supplyItem> m_supplyPositions;

	class iconItem
	{
	public:
		iconItem(int iType, const Vector& pos, float flLife, bool bEnabled)
		{
			this->type = iType;
			this->pos = pos;
			this->flLife = flLife;
			this->enabled = bEnabled;
		}

	public:
		int type;
		Vector pos;
		float flLife;
		bool enabled;

	public:
		inline int operator==(const iconItem& src) const { return src.type == this->type && src.enabled == this->enabled && src.pos == this->pos && src.flLife == this->flLife; }
	};

	std::unordered_map<int, std::vector<iconItem>> m_iconItems;


	UniqueTexture m_iTrap;

};
}