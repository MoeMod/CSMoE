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
#include "draw_util.h"
#include "calcscreen.h"
#include "com_model.h"

#include "events.h"
#include "triangleapi.h"
#include <r_efx.h>


DECLARE_MESSAGE(m_HeadIcon, HeadIcon)

enum
{
	HUMAN_SKILL_KNIFE2X,
	HUMAN_SKILL_HEADSHOT,
	ZOMBIE_SKILL_HEAL,
	ZOMBIE_SKILL_HEAL_HEAD,
	CANNON_FLAME_BURN
};

struct CHudHeadIconItem
{
	int iPlayer;
	int iType;
};

CHudHeadIconItem rgIcon;

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
	
	return 1;
}

void CHudHeadIcon::Shutdown(void)
{
	
}

int CHudHeadIcon:: Draw( float flTime )
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL)  )
		return 1;
	
	return 1;
}

// This message handler may be better off elsewhere
int CHudHeadIcon::MsgFunc_HeadIcon( const char *pszName, int iSize, void *pbuf )
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL))
		return 1;

	m_iFlags |= HUD_DRAW;

	BufferReader reader(pszName, pbuf, iSize);

	rgIcon.iType = reader.ReadByte();
	rgIcon.iPlayer = reader.ReadShort();

	switch (rgIcon.iType)
	{
	case HUMAN_SKILL_KNIFE2X:
	{
		R_AttachTentToEntity(rgIcon.iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zb_meleeup.spr"),
			Vector(0.0, 0.0, 30.0), 4.0, TRUE, FTENT_FADEOUT | FTENT_SPRANIMATE | FTENT_PERSIST | FTENT_PLYRATTACHMENT | FTENT_SPRANIMATELOOP, 1.0);
		break;
	}
	case HUMAN_SKILL_HEADSHOT:
	{
		R_AttachTentToEntity(rgIcon.iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zb_skill_headshot.spr"),
			Vector(0.0, 0.0, 30.0), 4.0, TRUE, FTENT_FADEOUT | FTENT_SPRANIMATE | FTENT_PERSIST | FTENT_PLYRATTACHMENT | FTENT_SPRANIMATELOOP, 1.0);
		break;
	}
	case ZOMBIE_SKILL_HEAL:
	{
		R_AttachTentToEntity(rgIcon.iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zombihealer.spr"),
			Vector(0.0, 0.0, 25.0), 1.0, TRUE, FTENT_FADEOUT | FTENT_SPRANIMATE | FTENT_PERSIST | FTENT_PLYRATTACHMENT | FTENT_SPRANIMATELOOP, 1.0);
		break;
	}
	case ZOMBIE_SKILL_HEAL_HEAD:
	{
		R_AttachTentToEntity(rgIcon.iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/zombiheal_head.spr"),
			Vector(0.0, 0.0, 25.0), 1.0, TRUE, FTENT_FADEOUT | FTENT_SPRANIMATE | FTENT_PERSIST | FTENT_PLYRATTACHMENT | FTENT_SPRANIMATELOOP, 1.0);
		break;
	}
	case CANNON_FLAME_BURN:
	{
		R_AttachTentToEntity(rgIcon.iPlayer, gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/flame_burn01.spr"),
			Vector(Com_RandomFloat(-5.0, 5.0), Com_RandomFloat(-5.0, 5.0), 25.0), 3, TRUE, FTENT_FADEOUT | FTENT_SPRANIMATE | FTENT_PERSIST | FTENT_PLYRATTACHMENT | FTENT_SPRANIMATELOOP, 0.3);
	}
	}

	return 1;
}
	

void CHudHeadIcon::R_AttachTentToEntity(int entity, int modelIndex, vec3_t offset, float life, int additive, int flags, float scale)
{
	model_s* model = gEngfuncs.pfnGetModelByIndex(modelIndex);

	if (model)
		R_AttachTentToEntity2(entity, model, offset, life, additive, flags, scale);
	else
		gEngfuncs.Con_Printf("No model %d!\n", modelIndex);
}

void CHudHeadIcon::R_AttachTentToEntity2(int entity, model_s* pModel, vec3_t offset, float life, int additive, int flags, float scale)
{
	cl_entity_t* ent = gEngfuncs.GetEntityByIndex(entity);

	if (!ent)
		return;

	if (!ent->index)
		return;

	TEMPENTITY* pTemp = gEngfuncs.pEfxAPI->CL_TempEntAlloc(ent->origin + offset, pModel);

	if (!pTemp)
	{
		gEngfuncs.Con_Printf("No temp ent.\n");

		return;
	}

	pTemp->clientIndex = entity;
	pTemp->entity.curstate.rendermode = kRenderNormal;
	pTemp->entity.baseline.renderamt = pTemp->entity.curstate.renderamt = 255;
	pTemp->entity.curstate.renderfx = kRenderFxNoDissipation;
	pTemp->entity.curstate.framerate = 1.0;
	pTemp->tentOffset = offset;
	pTemp->flags |= FTENT_PERSIST | FTENT_PLYRATTACHMENT;
	pTemp->die = gHUD.m_flTime + life;
	pTemp->entity.curstate.framerate = 10;

	if (offset.x || offset.y)
		pTemp->flags |= 0x400000;

	pTemp->entity.curstate.frame = 0;

	if (additive)
		pTemp->entity.curstate.rendermode = kRenderTransAdd;

	if (flags)
		pTemp->flags = flags;

	pTemp->entity.curstate.scale = scale;
}