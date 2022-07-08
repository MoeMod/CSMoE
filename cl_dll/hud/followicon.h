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

#include "r_texture.h"
namespace cl {
class CHudFollowIcon : public CHudBase
{
public:
	int Init(void) override;
	int VidInit(void) override;
	void Reset(void) override;
	int Draw(float time) override;
	void Shutdown(void) override;

public:
	int m_iBombTargetsNum;
	UniqueTexture m_pTexture_BombAB[2];
	UniqueTexture m_pTexture_Supplybox;
	Vector m_vecBombTargets[2];
};
}