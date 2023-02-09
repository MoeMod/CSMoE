/*
z4b_oppovivo.cpp
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#include "z4b_shovelex.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(z4b_shovelex, CKnifeShovelEX)


int CKnifeShovelEX::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iFlags = 0;
	p->iWeight = KNIFE_WEIGHT;

	return 1;
}

void CKnifeShovelEX::Precache()
{
	for (int i = 0; i < SHOVELEX_MAX_TYPE; i++)
	{
		PRECACHE_MODEL(V_CUSTOM_MODEL[i]);
		PRECACHE_MODEL(P_CUSTOM_MODEL[i]);
	}

	Base::Precache();
}

void CKnifeShovelEX::ItemPostFrame()
{
	if (m_pPlayer->pev->button & IN_RELOAD)
	{
		m_iWeaponState = 0;
		m_pPlayer->pev->button &= ~IN_RELOAD;

#ifndef CLIENT_DLL
		ItemInfo II;
		GetItemInfo(&II);


		MESSAGE_BEGIN(MSG_ONE, gmsgWeaponList2, NULL, m_pPlayer->pev);
		WRITE_STRING("z4b_shovelex");
		WRITE_SHORT(CBasePlayer::GetAmmoIndex(II.pszAmmo1));
		WRITE_BYTE(II.iMaxAmmo1);
		WRITE_SHORT(CBasePlayer::GetAmmoIndex(II.pszAmmo2));
		WRITE_BYTE(II.iMaxAmmo2);
		WRITE_BYTE(II.iSlot);
		WRITE_BYTE(II.iPosition);
		WRITE_SHORT(II.iId);
		WRITE_BYTE(II.iFlags);
		MESSAGE_END();
#endif
		Deploy();
		
		return;
	}

	return CBasePlayerWeapon::ItemPostFrame();
}

BOOL CKnifeShovelEX::Deploy()
{
	m_fMaxSpeed = 250;
	m_iSwing = 0;
	ResetRunIdle();

	if (DefaultDeploy(V_CUSTOM_MODEL[m_iWeaponState], P_CUSTOM_MODEL[m_iWeaponState], ANIM_DRAW, AnimExtension, UseDecrement() != FALSE))
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
		pev->body = m_pPlayer->m_bIsFemale ? 1 : 2;
		return TRUE;
	}
	return FALSE;
}

void CKnifeShovelEX::Spawn()
{
	pev->classname = MAKE_STRING("z4b_shovelex");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	m_iClip = WEAPON_NOCLIP;
	FallInit();
}

void CKnifeShovelEX::ChangeWeapon()
{
	m_iWeaponState = RANDOM_LONG(0, SHOVELEX_MAX_TYPE - 1);

#ifndef CLIENT_DLL
	ItemInfo II;
	GetItemInfo(&II);

	const char* pszName;
	switch (m_iWeaponState)
	{
	case 1:
		pszName = "z4b_kukriex"; break;
	case 2:
		pszName = "z4b_normalknife"; break;
	case 3:
		pszName = "z4b_hawlchicken"; break;
	case 4:
		pszName = "z4b_lightknife"; break;	
	default:
		pszName = "z4b_shovelex"; break;
	}

	MESSAGE_BEGIN(MSG_ONE, gmsgWeaponList2, NULL, m_pPlayer->pev);
	WRITE_STRING(pszName);
	WRITE_SHORT(CBasePlayer::GetAmmoIndex(II.pszAmmo1));
	WRITE_BYTE(II.iMaxAmmo1);
	WRITE_SHORT(CBasePlayer::GetAmmoIndex(II.pszAmmo2));
	WRITE_BYTE(II.iMaxAmmo2);
	WRITE_BYTE(II.iSlot);
	WRITE_BYTE(II.iPosition);
	WRITE_SHORT(II.iId);
	WRITE_BYTE(II.iFlags);
	MESSAGE_END();
#endif
}

void CKnifeShovelEX::Holster(int skiplocal)
{
	SetThink(nullptr);

	return CBasePlayerWeapon::Holster(skiplocal);
}

void CKnifeShovelEX::PrimaryAttack(void)
{
	m_iSwing = 0;
	SendWeaponAnim(pev->iuser1++ ? ANIM_SLASH1 : ANIM_SLASH2, UseDecrement() != FALSE);
	if (pev->iuser1)
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/shovelex/shovelex_Attack_1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	else
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/shovelex/shovelex_Attack_2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	pev->iuser1 %= 2;
	SetThink(&CKnifeShovelEX::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + 0.17s;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
	ResetRunIdle();
}

void CKnifeShovelEX::SecondaryAttack(void)
{
	SendWeaponAnim(RANDOM_LONG(0, 1)? ANIM_STAB1:ANIM_STAB2, UseDecrement() != FALSE);
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "CK_WPN/WEAPON/shovelex/Hero_ATTACK_3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	SetThink(&CKnifeShovelEX::DelaySecondaryAttack);
	pev->nextthink = gpGlobals->time + 0.87s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.4s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
	m_iSwing = 0;
	ResetRunIdle();
}

void CKnifeShovelEX::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack5(GetPrimaryAttackDamage(), 85, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/Mechanic/Mechanic_Hit.WAV", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/DualKukri/Hero_SWING.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif
}

void CKnifeShovelEX::DelaySecondaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack5(GetSecondaryAttackDamage(), 135, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
	case HIT_PLAYER:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/Mechanic/Mechanic_Hit.WAV", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_BODY, "CK_WPN/WEAPON/DualKukri/UI_SPECIALKILL2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CK_WPN/WEAPON/DualKukri/Hero_SWING.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		break;
	}
	}
#endif

	SetThink(nullptr);
}

void CKnifeShovelEX::WeaponIdle(void)
{
	ResetEmptySound();
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
		{
			if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(ANIM_RUN, UseDecrement() != FALSE);

				m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.8s;
			}

			m_flNextIdle = UTIL_WeaponTimeBase();
		}
		else
		{
			if (m_flNextIdle <= UTIL_WeaponTimeBase())
			{
				SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

				m_flNextIdle = UTIL_WeaponTimeBase() + 20s;
			}

			m_flNextRunIdle = UTIL_WeaponTimeBase();
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
	}

}

}
