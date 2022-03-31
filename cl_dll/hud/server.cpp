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
#include "com_model.h"
#include "r_studioint.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include <studio.h>
#include "events.h"
#include "r_efx.h"
#include "gamemode/mods_const.h"
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "cs_wpn/bte_weapons.h"

namespace cl {
	extern engine_studio_api_t IEngineStudio;

int CHud::MsgFunc_MPToCL(const char* pszName, int iSize, void* pbuf)
{
	int iType;
	BufferReader reader(pszName, pbuf, iSize);
	int arg1;
	int arg2;
	iType = reader.ReadByte();
	switch (iType)
	{
	case 1:
		{
			CBasePlayerWeapon* pActiveBTEWeapon = BTEClientWeapons().GetActiveWeaponEntity();
			if (pActiveBTEWeapon && pActiveBTEWeapon->m_iId == WEAPON_M95TIGER)
				pActiveBTEWeapon->pev->iuser1 = reader.ReadByte();
			break;
		}
	case 79:	// stop sound
		arg1 = reader.ReadShort();
		arg2 = reader.ReadShort();

		gEngfuncs.pEventAPI->EV_StopSound(arg1, arg2, reader.ReadString());

		break;
	}
	return 1;
}


void R_AttachTentToEntity(int entity, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode, float framerate)
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


void R_AttachTentToPlayer(int client, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale, int rendermode, float framerate)
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