/*
bufffiveseven.cpp
Copyright (C) 2019 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CBuffFiveSeven : public LinkWeaponTemplate< CBuffFiveSeven,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_FIVESEVEN>::template type,
            TPrecacheEvent,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_bufffiveseven.mdl";
    static constexpr const char *P_Model = "models/p_bufffiveseven.mdl";
    static constexpr const char *W_Model = "models/w_bufffiveseven.mdl";
    static constexpr const char *EventFile = "events/bufffiveseven.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_bufffiveseven";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 50;

	enum
	{
		ANIM_IDLEA = 0,
        ANIM_IDLEB,
		ANIM_IDLEC,
        ANIM_SHOOTA,
		ANIM_SHOOTA_EMPTY,
		ANIM_SHOOTB,
		ANIM_SHOOTB_EMPTY,
		ANIM_SHOOTC,
		ANIM_SHOOTC_EMPTY,
		ANIM_RELOADA,
		ANIM_RELOADB,
		ANIM_RELOADC,
		ANIM_DRAWA,
		ANIM_DRAWB,
		ANIM_DRAWC,
		ANIM_CHANGEAB,
		ANIM_CHANGEBC,
		ANIM_CHANGEBA,
		ANIM_CHANGECA,
		ANIM_SHOOT_SPECIALB,
		ANIM_SHOOT_SPECIALC,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.5f;
 
    static constexpr int DamageDefault = 46;
    static constexpr int DamageZB = 74;
    static constexpr int DamageZBS = 175;

    static constexpr float AccuracyDefault = 0.95;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 400.0f, 300.0f, 350.0f, 150.0f, 0.8f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50aeahe" , 90 }; }
#endif
	float GetArmorRatioModifier() override { return 1.5; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

	int m_iTargetId;
	int m_iAmmo2;
	int m_iExpIndex;
	int m_iExpIndex2;
	time_point_t m_flTimeNextCharge;
#ifndef CLIENT_DLL
	Vector m_vecExpPos;
#endif
	void Spawn()
	{
		m_iTargetId = 0;
		m_iSwing = 0;
		m_iAmmo2 = 0;
#ifndef CLIENT_DLL
		m_vecExpPos = g_vecZero;
#endif
		Base::Spawn();
	}

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "50aeahe";
		p->iMaxAmmo1 = 90;
		p->pszAmmo2 = "InfiniteAmmo";
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 50;
		p->iSlot = 1;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_BUFFFIVESEVEN;
		p->iFlags = 0;
		p->iWeight = FIVESEVEN_WEIGHT;

		return 1;
	}

	void Precache(void)
	{
		PRECACHE_MODEL("sprites/muzzleflash195.spr");
		PRECACHE_MODEL("sprites/muzzleflash196.spr");
		m_iExpIndex = PRECACHE_MODEL("sprites/bufffiveseven_exp.spr");
		m_iExpIndex2 = PRECACHE_MODEL("sprites/bufffiveseven_exp2.spr");
		PRECACHE_MODEL("models/bufffiveseven_field.spr");
		PRECACHE_MODEL("models/bufffiveseven_ef.spr");

		PRECACHE_SOUND("weapons/bufffiveseven_beep_AC.wav");
		PRECACHE_SOUND("weapons/bufffiveseven_beep_BC.wav");
		PRECACHE_SOUND("weapons/bufffiveseven_direct_AB.wav");
		PRECACHE_SOUND("weapons/bufffiveseven_direct_BC.wav");
		PRECACHE_SOUND("weapons/bufffiveseven_fire_loop.wav");
		PRECACHE_SOUND("weapons/bufffiveseven_red_skill_fx.wav");
		PRECACHE_SOUND("weapons/bufffiveseven_red_skill_start.wav");
		PRECACHE_SOUND("weapons/bufffiveseven_blue_skill_fx.wav");
		PRECACHE_SOUND("weapons/bufffiveseven_blue_skill_start.wav");
		//PRECACHE_SOUND("weapons/bufffiveseven-1.wav");
		PRECACHE_SOUND("weapons/bufffiveseven-1_exp.wav");

		Base::Precache();
	}

	void ItemPostFrame()
	{
		if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
		{
			m_flTimeNextCharge = invalid_time_point;

			if (m_iAmmo2 < m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
			{
				if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 100)
				{
					if (m_pPlayer->pev->weaponanim == ANIM_SHOOTB || m_pPlayer->pev->weaponanim == ANIM_SHOOTB_EMPTY)
					{
						SendWeaponAnim(ANIM_CHANGEBC, UseDecrement() != FALSE);

						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;

						EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/bufffiveseven_direct_BC.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
					}
				}
				else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 25)
				{
					if (m_pPlayer->pev->weaponanim == ANIM_SHOOTA || m_pPlayer->pev->weaponanim == ANIM_SHOOTA_EMPTY)
					{
						SendWeaponAnim(ANIM_CHANGEAB, UseDecrement() != FALSE);

						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;

						EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/bufffiveseven_direct_AB.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
					}
				}
			}
			else
			{
				if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 25)
				{
					if (m_pPlayer->pev->weaponanim == ANIM_SHOOT_SPECIALB)
					{
						SendWeaponAnim(ANIM_CHANGEBA, UseDecrement() != FALSE);

						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
					}
					else if (m_pPlayer->pev->weaponanim == ANIM_SHOOT_SPECIALC)
					{
						SendWeaponAnim(ANIM_CHANGECA, UseDecrement() != FALSE);

						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
					}
				}
			}
		}

		CBasePlayerWeapon::ItemPostFrame();
	}


	void SetChargeSkin()
	{
		int skin = floor(float(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]) / float(15));
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 100)
			skin = 7;

		pev->skin = skin;
	}

	void PrimaryAttack(void)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			BuffFiveSevenFire((1.5) * (1 - m_flAccuracy), 0.2s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			BuffFiveSevenFire((0.255) * (1 - m_flAccuracy), 0.2s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			BuffFiveSevenFire((0.075) * (1 - m_flAccuracy), 0.2s, FALSE);
		else
			BuffFiveSevenFire((0.15) * (1 - m_flAccuracy), 0.2s, FALSE);
	}


	void BuffFiveSevenFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		flCycleTime -= 0.05s;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy -= (0.275 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.25;

			if (m_flAccuracy > 0.92)
				m_flAccuracy = 0.92;
			else if (m_flAccuracy < 0.725)
				m_flAccuracy = 0.725;
		}

		m_flLastFire = gpGlobals->time;

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
		SetPlayerShieldAnim();
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

		Vector vecForward, vecUp, vecRight;
		vecForward = gpGlobals->v_forward;
		vecUp = gpGlobals->v_up;
		vecRight = gpGlobals->v_right;
		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 4096, 1, BULLET_PLAYER_57MM, 20, 0.885, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		int iAnim;
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 100)
			iAnim = m_iClip ? ANIM_SHOOTC : ANIM_SHOOTC_EMPTY;
		else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 25)
			iAnim = m_iClip ? ANIM_SHOOTB : ANIM_SHOOTB_EMPTY;
		else
			iAnim = m_iClip ? ANIM_SHOOTA : ANIM_SHOOTA_EMPTY;

		SendWeaponAnim(iAnim, UseDecrement() != FALSE);

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_vecExpPos.IsZero())
		{
			vecDir = vecForward + vecDir[0] * vecRight + vecDir[1] * vecUp;
			Vector vecEnd = m_vecExpPos - vecDir * 5;
			float flRadius = 1.0 * 39.37;
			bool bInWater = UTIL_PointContents(m_vecExpPos) == CONTENTS_WATER;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/bufffiveseven-1_exp.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 100)
			{
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(vecEnd.x);
				WRITE_COORD(vecEnd.y);
				WRITE_COORD(vecEnd.z);
				WRITE_SHORT(m_iExpIndex2);
				WRITE_BYTE(3);
				WRITE_BYTE(255);
				WRITE_BYTE(30);
				MESSAGE_END();
			}
			else
			{
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(TE_TEMPSPRITE);
				WRITE_COORD(vecEnd.x);
				WRITE_COORD(vecEnd.y);
				WRITE_COORD(vecEnd.z);
				WRITE_SHORT(m_iExpIndex);
				WRITE_BYTE(3);
				WRITE_BYTE(255);
				WRITE_BYTE(30);
				MESSAGE_END();
			}


			TraceResult tr;
			CBaseEntity* pEntity = NULL;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecEnd, flRadius)) != NULL)
			{
				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (bInWater && pEntity->pev->waterlevel == 0)
					continue;

				if (!bInWater && pEntity->pev->waterlevel == 3)
					continue;

				if (pEntity == m_pPlayer)
					continue;

				if(pEntity->entindex() == m_iTargetId)
					continue;

				UTIL_TraceLine(vecEnd, pEntity->pev->origin, dont_ignore_monsters, ENT(pEntity->pev), &tr);

				if (tr.flFraction == 1.0)
				{
					ClearMultiDamage();
					pEntity->TraceAttack(m_pPlayer->pev, GetDamage(), vecDir, &tr, DMG_BULLET | DMG_NEVERGIB);
					ApplyMultiDamage(pev, m_pPlayer->pev);
				}
			}
		}

		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
		m_flTimeNextCharge = gpGlobals->time + 1.0s;

		m_iAmmo2 = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 100)
		{
			if ((m_iSwing++ % 2))
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += 3;
#ifndef CLIENT_DLL
				MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
				WRITE_BYTE(CBasePlayer::GetAmmoIndex("bufffiveseven2"));
				WRITE_BYTE(3);
				MESSAGE_END();
#endif
			}
			else
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += 2;
#ifndef CLIENT_DLL
				MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
				WRITE_BYTE(CBasePlayer::GetAmmoIndex("bufffiveseven2"));
				WRITE_BYTE(2);
				MESSAGE_END();
#endif
			}
		}

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;

		if(m_iAmmo2 < m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 100)
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/bufffiveseven_beep_BC.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

		if (m_iAmmo2 < m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 25)
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/bufffiveseven_beep_AC.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		


		SetChargeSkin();

		m_pPlayer->pev->punchangle.x -= 2;
	}

	void SecondaryAttack(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 25)
			return;

		m_iAmmo2 = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 100)
		{
			//red stun maxspeed 1  knockback 0.8
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
			SendWeaponAnim(ANIM_SHOOT_SPECIALC, UseDecrement() != FALSE);		

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/bufffiveseven_red_skill_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

			
		}
		else
		{
			//blue  knockback 1.0
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] -= 25;
			SendWeaponAnim(ANIM_SHOOT_SPECIALB, UseDecrement() != FALSE);

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/bufffiveseven_blue_skill_start.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		}

		m_flTimeNextCharge = gpGlobals->time + 1.0s;

		SetChargeSkin();

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
	}

	BOOL Deploy()
	{
		m_flAccuracy = 0.92;
		m_fMaxSpeed = 250;

		int iAnim;
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 100)
			iAnim = ANIM_DRAWC;
		else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 25)
			iAnim = ANIM_DRAWB;
		else
			iAnim = ANIM_DRAWA;
		
		return DefaultDeploy(V_Model, P_Model, iAnim, "onehanded", UseDecrement() != FALSE);
		
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		int iAnim;
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 100)
			iAnim = ANIM_RELOADC;
		else if(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 25)
			iAnim = ANIM_RELOADB;
		else
			iAnim = ANIM_RELOADA;

		if (DefaultReload(MaxClip, iAnim, 2.2s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.7s;
			m_flAccuracy = 0.92;
		}
	}

	float GetBuffDamage() const
	{
		float flDamage = 1.0;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 30.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 85.0f;
#endif
		return flDamage;
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		int iAnim;
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 100)
			iAnim = ANIM_IDLEC;
		else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 25)
			iAnim = ANIM_IDLEB;
		else
			iAnim = ANIM_IDLEA;

		if (m_pPlayer->pev->weaponanim != iAnim)
			SendWeaponAnim(iAnim, UseDecrement() != FALSE);

	}


#ifndef CLIENT_DLL
	void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* tr)
	{
		int iOriginalPenetration = 1;
		if (iOriginalPenetration == iPenetration)
		{
			m_vecExpPos = tr->vecEndPos;
			if (gMultiDamage.pEntity)
				m_iTargetId = gMultiDamage.pEntity->entindex();
		}
	}
#endif

};
LINK_ENTITY_TO_CLASS(weapon_bufffiveseven, CBuffFiveSeven)
}