/*
wpn_thanatos11.cpp - Port of Zombie IV Mod
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

#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"
#include "weapons/RadiusDamage.hpp"
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
#ifndef CLIENT_DLL

#define THANATOS11_TIME_SLOWDOWN 3.0s
#define THANATOS11_SPEED_SLOWDOWN 86.0f

	class CThanatos112ndAmmo : public CBaseAnimating
	{
	public:
		CThanatos112ndAmmo()
		{
			m_pHit = nullptr;
			m_flRemoveTime = invalid_time_point;
			m_flDamageTime = invalid_time_point;
		}

		KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 0.6f }; }
		float GetArmorRatioModifier() { return 1.2; }
		int GetWeaponsId(void) { return WEAPON_THANATOS11; }

		void Spawn() override
		{
			Precache();

			SetThink(&CThanatos112ndAmmo::IgniteThink);
			SetTouch(&CThanatos112ndAmmo::SpearTouch);
			SET_MODEL(this->edict(), "models/thanatos11_scythe.mdl");

			pev->rendermode = kRenderNormal;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 255;
			pev->solid = SOLID_BBOX;
			pev->movetype = MOVETYPE_FLY;

			pev->frame = 0;
			pev->sequence = 0;

			ResetSequenceInfo();

			UTIL_SetSize(pev, { -2, -2, -2 }, { 2, 2, 2 });

			m_flRemoveTime = gpGlobals->time + 10.0s;

			pev->nextthink = gpGlobals->time + 0.1s;
		}

		void Precache() override
		{
			m_iBeamModelIndex = PRECACHE_MODEL("sprites/laserbeam.spr");
			PRECACHE_MODEL("models/thanatos11_scythe.mdl");

			m_iExpIndex = PRECACHE_MODEL("sprites/thanatos11_fire.spr");
			m_iEfIndex = PRECACHE_MODEL("sprites/thanatos11_scythe.spr");
			//PRECACHE_MODEL("sprites/thanatos_hit.spr");
			m_iCircleIndex = PRECACHE_MODEL("sprites/circle.spr");

			PRECACHE_SOUND("weapons/m1887-1.wav");
			PRECACHE_SOUND("weapons/thanatos11_shootb_hit.wav");
			PRECACHE_SOUND("weapons/thanatos11_explode.wav");
			PRECACHE_SOUND("weapons/thanatos11_count_start.wav");
			PRECACHE_SOUND("weapons/reload3.wav");
			PRECACHE_SOUND("weapons/reload3.wav");
		}

		void EXPORT SpearTouch(CBaseEntity* pOther)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_KILLBEAM);
			WRITE_SHORT(this->entindex());
			MESSAGE_END();

			if (!pOther)
				return;

			if (pOther->edict() == pev->owner)
				return;

			CBasePlayer* pOwner = (CBasePlayer*)GET_PRIVATE(pev->owner);

			if (!pOwner)
			{
				Remove();
				return;
			}

			if (pOwner->m_bIsZombie)
			{
				Remove();
				return;
			}

			float flDamage = GetDirectDamage();

			entvars_t* pevOwner = VARS(pev->owner);

			if ((pOther->IsPlayer() || pOther->Classify() == CLASS_PLAYER_ALLY) && pOther->IsAlive() && g_pGameRules->PlayerRelationship(pOwner, pOther) != GR_TEAMMATE)
			{
				m_pHit = pOther;

				TraceResult tr = UTIL_GetGlobalTrace();

				ClearMultiDamage();
				pOther->TraceAttack(pevOwner, flDamage, pev->velocity, &tr, DMG_BULLET | DMG_SLASH);
				ApplyMultiDamage(pev, pevOwner);

				pev->solid = SOLID_NOT;
				pev->movetype = MOVETYPE_FOLLOW;
				pev->aiment = pOther->edict();
				pev->velocity = g_vecZero;
				pev->nextthink = gpGlobals->time + 0.1s;
				m_flDamageTime = gpGlobals->time + 2s;
				
				UTIL_TempModel({ 0, 0, 40 }, g_vecZero, g_vecZero, m_iEfIndex, 20, 0, 150, 0, 0, 255, kRenderTransAdd, pOther, false, 0, 4, 6, 0x00010000);
			
				UTIL_TempModel({ 0, 0, 40 }, g_vecZero, g_vecZero, m_iCircleIndex, 20, 0, 50, 0, 0, 255, kRenderTransAdd, pOther, false, 0, 3, 10, 0);

				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/thanatos11_count_start.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

				EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/thanatos11_shootb_hit.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

				SetTouch(nullptr);
				pev->effects |= EF_NODRAW; // 0x80u
			}
			else
				Remove();		
		}

		void Update()
		{
			if (m_pHit)
			{
				if (m_pHit->IsAlive())
				{
					if (m_flDamageTime != invalid_time_point && m_flDamageTime < gpGlobals->time)
					{
						entvars_t* pevOwner = VARS(pev->owner);
						TraceResult tr;

						CBaseEntity* pEntity = NULL;
						while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pHit->pev->origin, 1.5 * 39.37)) != NULL)
						{
							if (pEntity->pev->takedamage == DAMAGE_NO)
								continue;

							if (pEntity->IsBSPModel())
								continue;

							if (pEntity->IsDormant())
								continue;

							Vector vecEnd = pEntity->BodyTarget(pev->origin);

							UTIL_TraceLine(m_pHit->pev->origin, vecEnd, dont_ignore_monsters, m_pHit->edict(), &tr);

							if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
								continue;

							if (pEntity->IsPlayer())
							{
								CBasePlayer* player = (CBasePlayer*)pEntity;

								player->m_tLockBuffTime = gpGlobals->time + THANATOS11_TIME_SLOWDOWN;
								player->m_flLockSpeed = THANATOS11_SPEED_SLOWDOWN;
								player->ResetMaxSpeed();
							}

							pEntity->TakeDamage(pev, pevOwner, GetExpDamage(), DMG_GENERIC | DMG_EXPLOSION);
						}

						EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/thanatos11_explode.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

						MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
						WRITE_BYTE(TE_TEMPSPRITE);
						WRITE_COORD(m_pHit->pev->origin.x);
						WRITE_COORD(m_pHit->pev->origin.y);
						WRITE_COORD(m_pHit->pev->origin.z);
						WRITE_SHORT(m_iExpIndex);
						WRITE_BYTE(8);
						WRITE_BYTE(255);
						WRITE_BYTE(15);
						MESSAGE_END();

						Remove();
					}
				}
				else
				{
					Remove();
				}
			}
		}

		void EXPORT FollowThink(void)
		{
			Update();
			pev->nextthink = gpGlobals->time + 0.032s;
		}

		void EXPORT IgniteThink(void)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(entindex());
			WRITE_SHORT(m_iBeamModelIndex);
			WRITE_BYTE(3);
			WRITE_BYTE(2);
			WRITE_BYTE(10);
			WRITE_BYTE(229);
			WRITE_BYTE(255);
			WRITE_BYTE(200);
			MESSAGE_END();

			SetThink(&CThanatos112ndAmmo::FollowThink);

			pev->nextthink = gpGlobals->time + 0.032s;
		}

		CBaseEntity* m_pHit;
		int m_iBeamModelIndex;
		int m_iEfIndex;
		int m_iCircleIndex;
		int m_iExpIndex;
		time_point_t m_flRemoveTime;
		time_point_t m_flDamageTime;

	private:
		float GetDirectDamage()
		{
			float flDamage = 1;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 1;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1;
			return flDamage;
		}

		float GetExpDamage()
		{
			float flDamage = 70;
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 435;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 1750;
			return flDamage;
		}

	protected:
		void Remove()
		{
			SetThink(nullptr);
			SetTouch(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			UTIL_Remove(this);
		}
	};

	LINK_ENTITY_TO_CLASS(thanatos112ndAmmo, CThanatos112ndAmmo)
#endif

    class CThanatos11 : public LinkWeaponTemplate< CThanatos11,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M3>::template type,
            TPrecacheEvent,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_thanatos11.mdl";
    static constexpr const char *P_Model = "models/p_thanatos11.mdl";
    static constexpr const char *W_Model = "models/w_thanatos11.mdl";
    static constexpr const char *EventFile = "events/thanatos11.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_thanatos11";
    static constexpr const char *AnimExtension = "shotgun";
    static constexpr int MaxClip = 15;
	static constexpr int DamageDefault = 20;
	static constexpr int DamageZB = 28;
	static constexpr int DamageZBS = 115;

	enum
	{
		ANIM_IDLEA = 0,
		ANIM_IDLEB1,
		ANIM_IDLEB2,
		ANIM_IDLEB_EMPTY,
		ANIM_SHOOTA,
		ANIM_SHOOTB,
		ANIM_SHOOTB_EMPTY,
		ANIM_CHANGEA,
		ANIM_CHANGEA_EMPTY,
		ANIM_CHANGEB,
		ANIM_CHANGEB_EMPTY,
		ANIM_INSERT,
		ANIM_AFTER_RELOAD,
		ANIM_START_RELOAD,
		ANIM_DRAW,
		ANIM_IDLEB_RELOAD
	};

#define WPNSTATE_THANATOS11_MODEB	1
#define THANATOS11_TIME_FIRERATE2 1.0s

#define THANATOS11_TIME_CHARGE 2.63s
#define THANATOS11_TIME_RETURN 1.63s
#define THANATOS11_TIME_REFILL 5.8s

#define THANATOS11_TIME_RELOAD_START 0.55s
#define THANATOS11_TIME_RELOAD_LOOP 0.35s

#define THANATOS11_2NDAMMO 3

    static constexpr float PlayerMaxSpeed = 240;
    KnockbackData KnockBack = { 1000.0f, 450.0f, 650.0f, 600.0f, 0.3f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M3); }
#endif
	float GetArmorRatioModifier() override { return 1.2; }
	WeaponCrosshairData GetCrosshairData() override
	{ 
		return { ACCURACY_AIR | ACCURACY_SPEED, 9, 4, 0 };
	}

	time_point_t m_flNextChargeCheck;
	duration_t m_flPumpTime;

	int ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	void Spawn()
	{
		m_flNextChargeCheck = gpGlobals->time + 1s;
		Base::Spawn();
	}

	void Precache() override
	{
		PRECACHE_SOUND("weapons/thanatos11_shootb.wav");
		PRECACHE_SOUND("weapons/thanatos11_shootb_empty.wav");

		m_iShellId = PRECACHE_MODEL("models/shotgunshell.mdl");

		return Base::Precache();
	}
    
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "buckshot";
		p->iMaxAmmo1 = 32;
		p->pszAmmo2 = "thanatos11SPAmmo";
		p->iMaxAmmo2 = 3;
		p->iMaxClip = 15;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_THANATOS11;
		p->iFlags = 0;
		p->iWeight = M3_WEIGHT;

		return 1;
	}

	BOOL Deploy(void)
	{
		m_iWeaponState &= ~WPNSTATE_THANATOS11_MODEB;
		if (DefaultDeploy("models/v_thanatos11.mdl", "models/p_thanatos11.mdl", ANIM_DRAW, "shotgun", UseDecrement() != FALSE, 0.1s))
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + THANATOS11_TIME_RETURN;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
			return TRUE;
		}
		return FALSE;
	}

	void ItemPostFrame()
	{
		if (m_flNextChargeCheck < gpGlobals->time)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = min(++m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType], 3);
	
			if ((m_iWeaponState & WPNSTATE_THANATOS11_MODEB) && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 1)
			{
				SendWeaponAnim(ANIM_IDLEB_RELOAD, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
			}
			m_flNextChargeCheck = gpGlobals->time + THANATOS11_TIME_REFILL;
			
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void SecondaryAttack()
	{
		if (m_iWeaponState & WPNSTATE_THANATOS11_MODEB)
		{
			m_iWeaponState &= ~WPNSTATE_THANATOS11_MODEB;

			SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] ? ANIM_CHANGEB : ANIM_CHANGEB_EMPTY, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + THANATOS11_TIME_RETURN;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + THANATOS11_TIME_RETURN;
			m_flNextChargeCheck += THANATOS11_TIME_RETURN;
			m_fInSpecialReload = 0;
		}
		else
		{
			m_iWeaponState |= WPNSTATE_THANATOS11_MODEB;

			SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] ? ANIM_CHANGEA : ANIM_CHANGEA_EMPTY, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + THANATOS11_TIME_CHARGE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + THANATOS11_TIME_CHARGE;
			m_flNextChargeCheck += THANATOS11_TIME_CHARGE;
			m_fInSpecialReload = 0;
		}
	}

	void PrimaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_THANATOS11_MODEB)
		{
			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
			{
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + THANATOS11_TIME_CHARGE;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + THANATOS11_TIME_CHARGE;
				m_flNextChargeCheck += THANATOS11_TIME_CHARGE;

				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + THANATOS11_TIME_FIRERATE2;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + THANATOS11_TIME_FIRERATE2;
				SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] ? ANIM_SHOOTB : ANIM_SHOOTB_EMPTY, UseDecrement() != FALSE);

				if(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
					EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/thanatos11_shootb.wav", VOL_NORM, 0.25, 0, PITCH_NORM);
				else
					EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/thanatos11_shootb_empty.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_ATTACK1);

				Vector vecAngle = m_pPlayer->pev->v_angle;
				vecAngle[0] = 360.0 - vecAngle[0];
				UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
				Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
				CThanatos112ndAmmo* pEnt = static_cast<CThanatos112ndAmmo*>(CBaseEntity::Create("thanatos112ndAmmo", vecSrc, vecAngle, ENT(m_pPlayer->pev)));

				if (pEnt)
				{
					pEnt->m_iTeam = m_pPlayer->m_iTeam;
					pEnt->pev->velocity = gpGlobals->v_forward * 1800;
				}
#endif
				if (m_pPlayer->pev->flags & FL_ONGROUND)
					m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 4, 8);
				else
					m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 12);
			}
			return;
		}


		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
		}

		if (m_iClip <= 0)
		{
			Reload();

			if (m_iClip == 0)
				PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			return;
		}

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(9, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0683, 0.0683, 0), 3000, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, FALSE, FALSE);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.2s;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.775s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.775s;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
		else
			m_flTimeWeaponIdle = 0.7s;

		m_fInSpecialReload = 0;

		if (m_pPlayer->pev->flags & FL_ONGROUND)
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 4, 8);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 12);

		m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.6s;
	}

	void Reload(void)
	{
		if (m_iWeaponState & WPNSTATE_THANATOS11_MODEB)
			return;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == MaxClip)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (!m_fInSpecialReload)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			SendWeaponAnim(ANIM_START_RELOAD, UseDecrement() != FALSE);

			m_fInSpecialReload = 1;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + THANATOS11_TIME_RELOAD_START;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + THANATOS11_TIME_RELOAD_START;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + THANATOS11_TIME_RELOAD_START;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + THANATOS11_TIME_RELOAD_START;
		}
		else if (m_fInSpecialReload == 1)
		{
			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_fInSpecialReload = 2;
			SendWeaponAnim(ANIM_INSERT, UseDecrement() != FALSE);

			m_flNextReload = UTIL_WeaponTimeBase() + THANATOS11_TIME_RELOAD_LOOP;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + THANATOS11_TIME_RELOAD_LOOP;
		}
		else
		{
			m_iClip++;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			m_fInSpecialReload = 1;
			m_pPlayer->ammo_buckshot--;
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
			m_flPumpTime = zero_duration;

		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			if (m_iWeaponState & WPNSTATE_THANATOS11_MODEB)
			{
				if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
				{
					SendWeaponAnim(ANIM_IDLEB_EMPTY, UseDecrement() != FALSE);
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
				}					
				else
				{
					if (RANDOM_LONG(0, 1))
					{
						SendWeaponAnim(ANIM_IDLEB1, UseDecrement() != FALSE);
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
					}
					else
					{
						SendWeaponAnim(ANIM_IDLEB2, UseDecrement() != FALSE);
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5s;
					}				
				}
			}
			else
			{
				if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
				{
					Reload();
				}
				else if (m_fInSpecialReload != 0)
				{
					if (m_iClip != MaxClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
					{
						Reload();
					}
					else
					{
						SendWeaponAnim(ANIM_AFTER_RELOAD, UseDecrement() != FALSE);

						m_fInSpecialReload = 0;
						m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
					}
				}
				else
				{
					SendWeaponAnim(ANIM_IDLEA, UseDecrement() != FALSE);
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
				}
					
			}
				
		}
	}


};
LINK_ENTITY_TO_CLASS(weapon_thanatos11, CThanatos11)
}