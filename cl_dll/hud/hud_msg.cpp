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
//
//  hud_msg.cpp
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"
#include "rain.h"
#include "com_model.h"
#include "studio.h"
#include "studio_util.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "com_weapons.h"

#include <cstring>

#include "events.h"

#include "gamemode/mods_const.h"

extern float g_flRoundTime;

/// USER-DEFINED SERVER MESSAGE HANDLERS

int CHud :: MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf )
{
	// clear all hud data
	HUDLIST *pList = m_pHudList;

	while ( pList )
	{
		if ( pList->p )
			pList->p->Reset();
		pList = pList->pNext;
	}

	// reset sensitivity
	m_flMouseSensitivity = 0;

	// reset concussion effect
	m_iConcussionEffect = 0;

	return 1;
}

void CAM_ToFirstPerson(void);

int CHud :: MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf )
{
	CAM_ToFirstPerson();
	return 1;
}

int CHud :: MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
	// prepare all hud data
	HUDLIST *pList = m_pHudList;

	while (pList)
	{
		if ( pList->p )
			pList->p->InitHUDData();
		pList = pList->pNext;
	}

	g_iFreezeTimeOver = 0;

	memset( g_PlayerExtraInfo, 0, sizeof(g_PlayerExtraInfo) );

	ResetRain();

	// reset round time
	g_flRoundTime   = 0.0f;

	// reinitialize models. We assume that server already precached all models.
	g_iRShell       = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/rshell.mdl" );
	g_iPShell       = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/pshell.mdl" );
	g_iShotgunShell = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/shotgunshell.mdl" );
	g_iBlackSmoke   = gEngfuncs.pEventAPI->EV_FindModelIndex( "sprites/black_smoke4.spr" );

	return 1;
}


int CHud :: MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	//m_Teamplay = reader.ReadByte();
	m_Teamplay = 1;

	m_iModRunning = static_cast<GameMode_e>(reader.ReadByte());

	if (m_iModRunning == MOD_SINGLEPLAY || m_iModRunning == MOD_MULTIPLAY)
		return 1;

	reader.ReadByte();
	m_Scoreboard.m_iTeamScore_Max = reader.ReadByte();
	reader.ReadByte();

	// reset mod-specific settings
	gHUD.m_ZB2.m_iFlags &= ~HUD_ACTIVE;
	gHUD.m_ZB3.m_iFlags &= ~HUD_ACTIVE;
	gHUD.m_ZBS.m_iFlags &= ~HUD_ACTIVE;

	switch (m_iModRunning)
	{
	case MOD_NONE:
	{
		int iBombTargetsNum = reader.ReadByte();
		iBombTargetsNum = min(iBombTargetsNum, 2);

		m_FollowIcon.m_iBombTargetsNum = iBombTargetsNum;
		for (int i = 0; i < iBombTargetsNum; ++i)
		{
			float x = reader.ReadCoord();
			float y = reader.ReadCoord();
			float z = reader.ReadCoord();
			m_FollowIcon.m_vecBombTargets[i] = { x,y,z };
		}
		break;
	}
	case MOD_TDM:
	{
		
		break;
	}
	case MOD_DM:
	{
		break;
	}
	
	case MOD_ZB3:
	{
		gHUD.m_ZB3.m_iFlags |= HUD_ACTIVE;
		// dont break, continue to ZB2...
		//[fallthrough]];
	}
	case MOD_ZB2:
	{
		gHUD.m_ZB2.m_iFlags |= HUD_ACTIVE;
		// dont break, continue to ZB1...
		//[fallthrough]];
	}
	case MOD_ZB1:
	{
		break;
	}
	case MOD_ZBS:
	{
		m_Teamplay = false;
		gHUD.m_ZBS.m_iFlags |= HUD_ACTIVE;
		break;
	}
	default:
		break;
	}

	return 1;
}

int CHud :: MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );
	m_iConcussionEffect = reader.ReadByte();
	if (m_iConcussionEffect)
		this->m_StatusIcons.EnableIcon("dmg_concuss",255,160,0);
	else
		this->m_StatusIcons.DisableIcon("dmg_concuss");
	return 1;
}

int CHud::MsgFunc_ShadowIdx(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );

	int idx = reader.ReadByte();
	g_StudioRenderer.StudioSetShadowSprite(idx);
	return 1;
}
