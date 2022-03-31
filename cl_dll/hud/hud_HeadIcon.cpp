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
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <studio.h>
#include "r_studioint.h"
#include "draw_util.h"
#include "calcscreen.h"
#include "com_model.h"

#include "events.h"
#include "r_efx.h"

#include "gamemode/mods_const.h"

namespace cl {
	extern engine_studio_api_t IEngineStudio;

DECLARE_MESSAGE(m_HeadIcon, HeadIcon)
#define MAXPLAYER 32
enum
{
	HUMAN_SKILL_KNIFE2X,
	HUMAN_SKILL_HEADSHOT,
	ZOMBIE_SKILL_HEAL,
	ZOMBIE_SKILL_HEAL_HEAD,
	CANNON_FLAME_BURN,
	HUNTBOW_DMGREITERATION,
	HUNTBOW_MARKZOMBIE,
	TELEPORT_MARKEF,
	HOLYBOMB_BURN,
	LANCE_HIT,
	ZOMBIE_SKILL_PILE,
};

struct CHudHeadIconItem
{
	int iPlayer;
	int iType;
	int iuser1;
	int iId;
};

CHudHeadIconItem rgIconList[MAXPLAYER + 1];

int CHudHeadIcon:: Init( void )
{
	gHUD.AddHudElem( this );
	m_iFlags |= HUD_DRAW;
	HOOK_MESSAGE(HeadIcon);

	return 1;
}

void CHudHeadIcon::Reset(void)
{
	VidInit();
}

int CHudHeadIcon:: VidInit( void )
{
	if (!m_pTexture_Zombie_s)
		R_InitTexture(m_pTexture_Zombie_s, "resource/helperhud/zombie_s");
	return 1;
}

void CHudHeadIcon::Shutdown(void)
{
	m_pTexture_Zombie_s = nullptr;
}

int CHudHeadIcon:: Draw( float flTime )
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL)  )
		return 1;

	if (gHUD.m_iModRunning == MOD_ZB2 || gHUD.m_iModRunning == MOD_ZB3 || gHUD.m_iModRunning == MOD_ZBZ)
	{
		for (int i = 0; i < MAXPLAYER; i++)
		{
			if (rgIconList[i].iType == HUNTBOW_MARKZOMBIE)
			{
				if (rgIconList[i].iuser1 == 1)
				{
					if (rgIconList[i].iId == 0)
						break;
					cl_entity_t* ent = gEngfuncs.GetEntityByIndex(rgIconList[i].iPlayer);
					float xyScreen[2];
					if (CalcScreen(ent->origin, xyScreen))
					{
						m_pTexture_Zombie_s->Draw2DQuadScaled(xyScreen[0] - 18, xyScreen[1] - 18, xyScreen[0] + 19, xyScreen[1] + 19);

						char szBuffer[16];
						sprintf(szBuffer, "[%im]", static_cast<int>((ent->origin - gHUD.m_vecOrigin).Length() / 42.0f));

						int textlen = DrawUtils::HudStringLen(szBuffer);
						int r, g, b;
						DrawUtils::UnpackRGB(r, g, b, RGB_YELLOWISH);
						DrawUtils::DrawHudString(xyScreen[0] - textlen * 0.5f, xyScreen[1] + 25, gHUD.m_scrinfo.iWidth, szBuffer, r, g, b);
					}
				}
				else
				{
					rgIconList[i].iId = 0;
					rgIconList[i].iPlayer = 0;
				}
			}
		}
	}

	return 1;
}

// This message handler may be better off elsewhere
int CHudHeadIcon::MsgFunc_HeadIcon( const char *pszName, int iSize, void *pbuf )
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL))
		return 1;

	m_iFlags |= HUD_DRAW;

	int i;
	for (i = 0; i < MAXPLAYER; i++)
	{
		if (rgIconList[i].iId == 0)
			break;
	}
	if (i == MAXPLAYER)
	{ // move the rest of the list forward to make room for this item
		memmove(rgIconList, rgIconList + 1, sizeof(CHudHeadIconItem) * MAXPLAYER);
		i = MAXPLAYER - 1;
	}

	BufferReader reader(pszName, pbuf, iSize);

	rgIconList[i].iType = reader.ReadByte();
	rgIconList[i].iPlayer = reader.ReadShort();
	rgIconList[i].iuser1 = reader.ReadByte();
	rgIconList[i].iId = 1;

	int iModelIndex = 0;
	int flags = FTENT_PERSIST | FTENT_FADEOUT | FTENT_SPRANIMATELOOP;
	switch (rgIconList[i].iType)
	{
	case HUMAN_SKILL_KNIFE2X:
	{
		R_AttachTentToPlayer(rgIconList[i].iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zb_meleeup.spr"),
			Vector(0.0, 0.0, 30.0), 10.0, TRUE, flags, 1.0);
		break;
	}
	case HUMAN_SKILL_HEADSHOT:
	{
		R_AttachTentToPlayer(rgIconList[i].iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zb_skill_headshot.spr"),
			Vector(0.0, 0.0, 30.0), rgIconList[i].iuser1 ? 5.5 : 4.5, TRUE, flags, 1.0);
		break;
	}
	case ZOMBIE_SKILL_HEAL:
	{
		R_AttachTentToPlayer(rgIconList[i].iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zombihealer.spr"),
			Vector(0.0, 0.0, 25.0), 1.0, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
		break;
	}
	case ZOMBIE_SKILL_HEAL_HEAD:
	{
		R_AttachTentToPlayer(rgIconList[i].iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zombiheal_head.spr"),
			Vector(0.0, 0.0, 25.0), 1.0, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
		break;
	}
	case CANNON_FLAME_BURN:
	{
		R_AttachTentToPlayer(rgIconList[i].iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_burn01.spr"),
			Vector(Com_RandomFloat(-5.0, 5.0), Com_RandomFloat(-5.0, 5.0), 0.0), 3, TRUE, flags, 0.3, kRenderTransAdd, 10.0);
		break;
	}
	case HUNTBOW_DMGREITERATION:
	{
		switch (rgIconList[i].iuser1)
		{
		case 1: {iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/dmgreiteration01.spr"); break; }
		case 2: {iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/dmgreiteration02.spr"); break; }
		case 3: {iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/dmgreiteration03.spr"); break; }
		case 4: {iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/dmgreiteration04.spr"); break; }
		case 5: {iModelIndex = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/dmgreiteration05.spr"); break; }
		}
		R_AttachTentToPlayer(rgIconList[i].iPlayer, iModelIndex,
			Vector(0.0, 0.0, 40.0), 1.0, TRUE, flags, 1.0);
		break;
	}
	case HUNTBOW_MARKZOMBIE:
	{
		break;
	}
	case TELEPORT_MARKEF:
	{
		flags = FTENT_PERSIST;
		if (rgIconList[i].iuser1)
			R_AttachTentToEntity(rgIconList[i].iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_teleportzombieout.spr"),
				Vector(0.0, 0.0, 0.0), 2.0, TRUE, flags, 0.8, kRenderTransAdd, 7.5);
		else
			R_AttachTentToPlayer(rgIconList[i].iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_teleportzombie.spr"),
				Vector(0.0, 0.0, 0.0), 2.0, TRUE, flags, 0.8, kRenderTransAdd, 7.5);
		break;
	}
	case HOLYBOMB_BURN:
	{
		float fLife = rgIconList[i].iuser1;
		R_AttachTentToPlayer(rgIconList[i].iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/holybomb_burn.spr"),
			Vector(0.0, 0.0, 30.0), fLife, TRUE, flags, 0.9, kRenderTransAdd, 10.0);
		break;
	}
	case LANCE_HIT:
	{
		flags = FTENT_PERSIST;
		R_AttachTentToPlayer(rgIconList[i].iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/ef_lance_hit.spr"),
			Vector(0.0, 0.0, 0.0), 0.18, TRUE, flags, 1.0, kRenderTransAdd, 10.0);
		break;
	}
	case ZOMBIE_SKILL_PILE:
	{
		R_AttachTentToPlayer(rgIconList[i].iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zbt_slow.spr"),
			Vector(0.0, 0.0, 40.0), 3.0, TRUE, flags, 1.0, kRenderTransAdd, 1.0);
		break;
	}
	}

	return 1;
}


void CHudHeadIcon::R_AttachTentToEntity(int entity, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode, float framerate)
{
	if (!modelIndex)
	{
		gEngfuncs.Con_Printf("No model %d!\n", modelIndex);
		return;
	}

	cl_entity_t* pEnity = gEngfuncs.GetEntityByIndex(entity);
	if (!pEnity)
	{
		gEngfuncs.Con_Printf("Couldn't get ClientEntity for %i\n", entity);
		return;
	}

	TEMPENTITY* pTemp = gEngfuncs.pEfxAPI->R_DefaultSprite(pEnity->origin + offset, modelIndex, 1.0);
	if (!pTemp)
	{
		gEngfuncs.Con_Printf("No temp ent.\n");

		return;
	}

	pTemp->entity.curstate.framerate = framerate;
	pTemp->entity.baseline.renderamt = pTemp->entity.curstate.renderamt = 255;
	pTemp->tentOffset = offset;
	pTemp->flags = FTENT_PLYRATTACHMENT | FTENT_SPRANIMATE | flags;
	pTemp->clientIndex = entity;
	pTemp->entity.curstate.renderfx = kRenderFxNoDissipation;
	pTemp->entity.curstate.scale = scale;
	pTemp->entity.curstate.rendermode = rendermode;
	if (additive)
		pTemp->entity.curstate.rendermode = kRenderTransAdd;
	pTemp->die = gHUD.m_flTime + life;

}


void CHudHeadIcon::R_AttachTentToPlayer(int client, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode, float framerate)
{
	if (!modelIndex)
	{
		gEngfuncs.Con_Printf("No model %d!\n", modelIndex);
		return;
	}

	if (client <= 0 || client > gEngfuncs.GetMaxClients())
	{
		gEngfuncs.Con_Printf("Bad client %i in R_AttachTentToEntity()!\n", client);
		return;
	}

	cl_entity_t* pClient = gEngfuncs.GetEntityByIndex(client);
	if (!pClient)
	{
		gEngfuncs.Con_Printf("Couldn't get ClientEntity for %i\n", client);
		return;
	}

	TEMPENTITY* pTemp = gEngfuncs.pEfxAPI->R_DefaultSprite(pClient->origin + offset, modelIndex, 1.0);
	if (!pTemp)
	{
		gEngfuncs.Con_Printf("No temp ent.\n");

		return;
	}

	pTemp->entity.curstate.framerate = framerate;
	pTemp->entity.baseline.renderamt = pTemp->entity.curstate.renderamt = 255;
	pTemp->tentOffset = offset;
	pTemp->flags = FTENT_PLYRATTACHMENT | FTENT_SPRANIMATE | flags;
	pTemp->clientIndex = client;
	pTemp->entity.curstate.renderfx = kRenderFxNoDissipation;
	pTemp->entity.curstate.scale = scale;
	pTemp->entity.curstate.rendermode = rendermode;
	if (additive)
		pTemp->entity.curstate.rendermode = kRenderTransAdd;
	pTemp->die = gHUD.m_flTime + life;

}

}