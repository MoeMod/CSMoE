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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapons/KnifeAttack.h"
#include "csgo_zeus.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
hit_result_t CZeus::ZeusAttack(Vector vecSrc, Vector vecDir, float flDamage, float flRadius, int bitsDamageType,
	entvars_t* pevInflictor, entvars_t* pevAttacker)
{
	TraceResult tr;
	hit_result_t result = HIT_NONE;

	vecSrc.z += 1;

	if (!pevAttacker)
		pevAttacker = pevInflictor;

	Vector vecEnd = vecSrc + vecDir.Normalize() * flRadius;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pevAttacker), &tr);

	if (tr.flFraction < 1) {
		CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
		if (pHit && pHit->pev->takedamage != DAMAGE_NO) 
		{
			if(pHit->IsBSPModel())
				result = HIT_WALL;
			else
			{
				const float flAdjustedDamage = flDamage * tr.flFraction;
				tr.iHitgroup = HITGROUP_CHEST;
				ClearMultiDamage();
				pHit->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
				ApplyMultiDamage(pevInflictor, pevAttacker);
				result = HIT_PLAYER;
			}	
		}

		float flVol = 1;
		BOOL fHitWorld = TRUE;
		if (pHit && pHit->Classify() != CLASS_NONE && pHit->Classify() != CLASS_MACHINE) {
			flVol = 0.1f;
			fHitWorld = FALSE;
		}

		if (fHitWorld) {
			result = HIT_WALL;
		}
	}
	if (result != HIT_NONE)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTPOINT);
		WRITE_SHORT(ENTINDEX(m_pPlayer->edict()) | 0x1000);
		WRITE_COORD(tr.vecEndPos.x);
		WRITE_COORD(tr.vecEndPos.y);
		WRITE_COORD(tr.vecEndPos.z);
		WRITE_SHORT(m_iSprBeam);
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(10); // life
		WRITE_BYTE(20);  // width
		WRITE_BYTE(7);   // noise
		WRITE_BYTE(50);
		WRITE_BYTE(20);
		WRITE_BYTE(255);
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(25);		// speed
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTPOINT);
		WRITE_SHORT(ENTINDEX(m_pPlayer->edict()) | 0x1000);
		WRITE_COORD(tr.vecEndPos.x);
		WRITE_COORD(tr.vecEndPos.y);
		WRITE_COORD(tr.vecEndPos.z);
		WRITE_SHORT(m_iSprBeam);
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(10); // life
		WRITE_BYTE(13); // width
		WRITE_BYTE(30);   // noise
		WRITE_BYTE(50);
		WRITE_BYTE(20);
		WRITE_BYTE(255);
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(25);		// speed
		MESSAGE_END();
	}

	return result;
}
#endif
LINK_ENTITY_TO_CLASS(csgo_zeus, CZeus)

void CZeus::Spawn(void)
{
	pev->classname = MAKE_STRING("csgo_zeus");

	Precache();
	m_iId = WEAPON_ELITE;
	SET_MODEL(ENT(pev), "models/w_zeus.mdl");
	m_iClip = -1;
	m_iDefaultAmmo = 1;
	FallInit();
}

void CZeus::Precache(void)
{
	PRECACHE_MODEL("models/v_zeus.mdl");
	PRECACHE_MODEL("models/w_zeus.mdl");
	PRECACHE_MODEL("models/p_zeus.mdl");
	m_iSprBeam = PRECACHE_MODEL("sprites/zbeam1.spr");
	PRECACHE_SOUND("weapons/csgo_ports/zeus/zeus_draw.wav");
	PRECACHE_SOUND("weapons/csgo_ports/zeus/zeus_hit.wav");
	PRECACHE_SOUND("weapons/csgo_ports/zeus/zeus_shoot,wav");
}

int CZeus::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ZeusAmmo";
	p->iMaxAmmo1 = 1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 2;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_ELITE;
	p->iFlags = ITEM_FLAG_EXHAUSTIBLE;
	p->iWeight = FLASHBANG_WEIGHT;

	return 1;
}

BOOL CZeus::Deploy(void)
{
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/csgo_ports/zeus/zeus_draw.wav", 0.3, 2.4);
	if (DefaultDeploy("models/v_zeus.mdl", "models/p_zeus.mdl", ANIM_DRAW, "onehanded", UseDecrement() != FALSE))
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.1s;
		return TRUE;
	}

	return FALSE;
}


void CZeus::PrimaryAttack(void)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 130;
	float fDamage = GetPrimaryAttackDamage();

	SendWeaponAnim(ANIM_SHOOT, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
	switch (ZeusAttack(vecSrc, gpGlobals->v_forward, fDamage, 130, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
	{
	case HIT_NONE:
	{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/csgo_ports/zeus/zeus_shoot.wav", VOL_NORM, ATTN_NORM, 0, 94);
		break;
	}
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/csgo_ports/zeus/zeus_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		fDidHit = TRUE;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/csgo_ports/zeus/zeus_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
		fDidHit = TRUE;
		break;
	}
	}
#endif
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
#ifndef CLIENT_DLL
	m_pPlayer->DropPlayerItem(STRING(this->pev->classname));
#endif

}

#ifndef CLIENT_DLL
void CZeus::AttachToPlayer(CBasePlayer* pPlayer)
{
	CBasePlayerWeapon::AttachToPlayer(pPlayer);

	/*// find the linked weapon
	while (!m_pLink)
	{
		for (int i = 0; i < MAX_ITEM_TYPES; ++i)
		{
			CBasePlayerItem* pItem = m_pPlayer->m_rgpPlayerItems[i];
			while (pItem != NULL)
			{
				if (!Q_stricmp(pItem->pszName(), "weapon_scarh"))
				{
					m_pLink = pItem;

					m_pLink->m_iSwing = 1;
					m_iSwing = 0;
				}

				pItem = pItem->m_pNext;
			}
		}
		if (!m_pLink)
			pPlayer->GiveNamedItem("weapon_scarh");

	}*/
}

void CZeus::ItemPostFrame()
{
	/*m_iSwing = 0;
	if (m_pLink)
		m_pLink->m_iSwing = 1;*/
	return CBasePlayerWeapon::ItemPostFrame();
}

CZeus::~CZeus()
{
	/*CBaseEntity* pOther = m_pLink;
	if (pOther && pOther->m_pLink == this)
	{
		pOther->m_pLink = m_pLink = nullptr;
		pOther->SUB_Remove();
	}*/
}
#endif

void CZeus::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		RetireWeapon();
		return;
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
	SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
}


float CZeus::GetPrimaryAttackDamage() const
{
	int flDamage = 125;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 1000;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 500;
#endif
	return flDamage;
}

int CZeus::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
		m_iClip = WEAPON_NOCLIP;
		m_iDefaultAmmo = 0;
		return TRUE;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CZeus::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

}
