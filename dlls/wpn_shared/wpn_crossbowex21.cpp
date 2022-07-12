/*
wpn_CCrossBowEX21.cpp - Port of Zombie IV Mod
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
#include "studio.h"

#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"
#include "weapons/RadiusDamage.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define WPNSTATE_CROSSBOWEX_WINGON	(1<<1)

    class CCrossBowEX21 : public LinkWeaponTemplate< CCrossBowEX21,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SG552>::template type,
            TPrecacheEvent,
            TDeployDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_crossbowex21.mdl";
    static constexpr const char *P_Model = "models/p_crossbowex21.mdl";
    static constexpr const char *W_Model = "models/w_crossbowex21.mdl";
    static constexpr const char *EventFile = "events/crossbowex21.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_crossbowex21";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 50;
	enum
	{
		ANIM_IDLE,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_WINGSTART,
		ANIM_WINGLOOP,
		ANIM_WINGEND,
		ANIM_WINGSHOOT1,
		ANIM_WINGSHOOT2,
		ANIM_WINGRELOAD,//USELESS
		ANIM_WINGSPECIALSHOT,
		ANIM_WINGRELOAD2

	};
    static constexpr float PlayerMaxSpeed = 205;
    static constexpr auto BulletType = BULLET_PLAYER_BUCKSHOT;
    static constexpr int Penetration = 1;
    KnockbackData KnockBack = { 1000.0f, 700.0f, 450.0f, 250.0f, 0.4f };

	time_point_t m_flTimePlayIdleSound;
	time_point_t m_flTimeHolster;
	time_point_t m_flTimeNextCharge;
	int m_iRecordAmmo;
	int m_iSummonModelIndex01;
	int m_iExpIndex;
	int m_iExpModelIndex;
	bool m_bChargeFinish;
	int m_iDefaultAmmo2;

	KnockbackData GetKnockBackData() override
	{
		//wingshoot 0.9
		if (m_iWeaponState & WPNSTATE_CCrossBowEX21_WINGON)
			return { 0.0f, 0.0f, 0.0f, 0.0f, 0.9f };
		else
			return { 1000.0f, 700.0f, 450.0f, 250.0f, 0.4f };
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_CCrossBowEX21" , 175 }; }
#endif
	float GetArmorRatioModifier() override { return 1.2; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 9, 4, 0 }; }

	void Spawn() override
	{
		m_bChargeFinish = false;
		m_iRecordAmmo = 0;
		m_iDefaultAmmo2 = 0;
		m_flTimeHolster = gpGlobals->time;
		m_flTimePlayIdleSound = invalid_time_point;
		Base::Spawn();
	}

	void Precache() override
	{
		PRECACHE_SOUND("weapons/CCrossBowEX21-2.wav");
		PRECACHE_SOUND("weapons/CCrossBowEX21_special_on.wav");
		PRECACHE_SOUND("weapons/CCrossBowEX21_loop.wav");
		PRECACHE_SOUND("weapons/CCrossBowEX21_loop_end.wav");
		PRECACHE_MODEL("models/CCrossBowEX21_bullet.mdl");
		m_iExpModelIndex = PRECACHE_MODEL("models/ef_CCrossBowEX21_explosion.mdl");

		PRECACHE_MODEL("sprites/muzzleflash163.spr");
		PRECACHE_MODEL("sprites/muzzleflash165.spr");
		PRECACHE_MODEL("sprites/muzzleflash166.spr");
		PRECACHE_MODEL("sprites/muzzleflash169.spr");
		PRECACHE_MODEL("sprites/muzzleflash171.spr");

		PRECACHE_MODEL("sprites/ef_CCrossBowEX21_particle.spr");	//hit wall
		PRECACHE_MODEL("sprites/ef_CCrossBowEX21_ring.spr");	//attachment[2] client
		PRECACHE_MODEL("sprites/ef_CCrossBowEX21_laserbeam.spr");
		m_iExpIndex = PRECACHE_MODEL("sprites/ef_CCrossBowEX21_explosion.spr");

		PRECACHE_MODEL("sprites/ef_CCrossBowEX21_charging2.spr");	//attachment[2]
		PRECACHE_MODEL("sprites/ef_CCrossBowEX21_charging.spr");	//attachment[4/5]

		m_iSummonModelIndex01 = PRECACHE_MODEL("sprites/ef_CCrossBowEX21_summon01.spr");
		
		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "CCrossBowEX21BuckShot";
		p->iMaxAmmo1 = 100;
		p->pszAmmo2 = "WingEnergy";
		p->iMaxAmmo2 = 100;
		p->iMaxClip = 50;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_CCrossBowEX21;
		p->iFlags = 0;
		p->iWeight = XM1014_WEIGHT;

		return 1;
	}

	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
	{
		if (m_iDefaultAmmo2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	void Holster(int skiplocal)
	{
		SetThink(nullptr);
		m_flTimeHolster = gpGlobals->time;
		RemoveWing();
		RemoveWingParticle();
		StopIdleSound();
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	BOOL Deploy(void)
	{
		m_iWeaponState &= ~WPNSTATE_CCrossBowEX21_WINGON;
		m_bChargeFinish = false;
		m_flTimePlayIdleSound = invalid_time_point;
#ifndef CLIENT_DLL
		if (m_flTimeHolster != invalid_time_point)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeHolster) / 0.25s;
		}
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;
#endif
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75s;
		return DefaultDeploy("models/v_CCrossBowEX21.mdl", "models/p_CCrossBowEX21.mdl", ANIM_DRAW, "shotgun", UseDecrement() != FALSE, 0.1s, 1.0s);
	}

	void ItemPostFrame() override
	{
		if (m_iWeaponState & WPNSTATE_CCrossBowEX21_WINGON)
		{
			if (m_flTimeNextCharge <= gpGlobals->time)
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
				m_flTimeNextCharge = gpGlobals->time + 0.15s;

				if (m_flTimePlayIdleSound != invalid_time_point && m_flTimePlayIdleSound < gpGlobals->time)
				{				
					m_flTimePlayIdleSound = invalid_time_point;
				}

				if (!m_bChargeFinish && m_iRecordAmmo - m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 20)
				{
					EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/CCrossBowEX21_special_on.wav", VOL_NORM, ATTN_NORM, 0, 94);
					m_flTimePlayIdleSound = gpGlobals->time + 1.0s;
					m_bChargeFinish = true;
#ifndef CLIENT_DLL
					if (m_pWing)
					{
						m_pWing->SendWingAnim(1);
					}
#endif
					//draw charge spr
					PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 7, 1, FALSE, TRUE);
				}

				if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
				{
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
					SecondaryAttack();
				}
			}
		}
		else
		{
			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 100)
			{
				if (m_flTimeNextCharge <= gpGlobals->time)
				{
					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
					m_flTimeNextCharge = gpGlobals->time + 0.3s;
				}
			}
		}

		return CBasePlayerWeapon::ItemPostFrame();
	}

	float GetMaxSpeed(void) override
	{
		if (m_iWeaponState & WPNSTATE_CCrossBowEX21_WINGON)
			return 280;

		return 240;
	}

	void CreateWing(void)
	{
#ifndef CLIENT_DLL
		if (m_pWing != nullptr)
			return;

		m_pWing = (CCrossBowEX21Wing*)Create("CCrossBowEX21_wing", m_pPlayer->pev->origin + Vector(0, 0, 0), g_vecZero, m_pPlayer->edict());

		if (m_pWing)
		{
			m_pWing->WingInit(m_pPlayer, this);
			m_pWing->m_iTeam = m_pPlayer->m_iTeam;
		}
#endif
		SetThink(nullptr);
	}

	void CreateWingParticle(void)
	{
#ifndef CLIENT_DLL
		if (m_pWingParticle != nullptr)
			return;

		m_pWingParticle = (CCrossBowEX21WingParticle*)Create("CCrossBowEX21_wingparticle", m_pPlayer->pev->origin + Vector(0, 0, 0), g_vecZero, m_pPlayer->edict());

		if (m_pWingParticle)
		{
			m_pWingParticle->WingParticleInit(m_pPlayer, this);
		}
#endif
	}

	void SecondaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_CCrossBowEX21_WINGON)
		{
			if (m_bChargeFinish)
			{
				SendWeaponAnim(ANIM_WINGSPECIALSHOT, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
				SpecialShoot();
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.2s;
#ifndef CLIENT_DLL
				if (m_pWing)
				{
					m_pWing->SecondaryAttack();
				}
#endif
			}
			else
			{
				SendWeaponAnim(ANIM_WINGEND, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
#ifndef CLIENT_DLL
				if (m_pWing)
				{
					m_pWing->RemoveWing();
				}
#endif
			}

			StopIdleSound();

			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 7, 1, TRUE, TRUE);

			m_iWeaponState &= ~WPNSTATE_CCrossBowEX21_WINGON;

			m_bChargeFinish = false;
			m_flTimeNextCharge = gpGlobals->time + 1.1s;
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "weapons/CCrossBowEX21_loop.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_flTimeNextCharge = gpGlobals->time + 1.1s;
			SendWeaponAnim(ANIM_WINGSTART, UseDecrement() != FALSE);
			m_iWeaponState |= WPNSTATE_CCrossBowEX21_WINGON;
			m_iRecordAmmo = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
#ifndef CLIENT_DLL
			UTIL_TempModel({ 0, 0, 50 }, pev->angles, g_vecZero, m_iSummonModelIndex01, 20, 0, 255, 0, 0, 255, kRenderTransAdd, m_pPlayer, false, 0, 5, 30, 0);
#endif
			SetThink(&CCrossBowEX21::CreateWing);
			pev->nextthink = gpGlobals->time + 1.0s;
			m_pPlayer->m_tLockBuffTime = invalid_time_point;
		}

		m_pPlayer->ResetMaxSpeed();
	}

	void PrimaryAttack(void)
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
		}

		if (m_iClip <= 0)
		{
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
		float flDamage, flDistance, flSpread;
		duration_t flCycleTime;
		int cShots;

		GetFireInfo(flDamage, flDistance, flSpread, cShots, flCycleTime);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		
#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(cShots, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(flSpread, flSpread, 0.0), flDistance, BULLET_PLAYER_CCrossBowEX21_BUCKSHOT, 0, flDamage);
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, flSpread, 7, m_vVecAiming.x * 100, m_iWeaponState & WPNSTATE_CCrossBowEX21_WINGON, FALSE);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25s;
		else
			m_flTimeWeaponIdle = 0.75s;
		
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 3);
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (m_bChargeFinish)
		{
			SecondaryAttack();
			return;
		}

		if (DefaultReload(50, m_iWeaponState & WPNSTATE_CCrossBowEX21_WINGON ? ANIM_WINGRELOAD2 : ANIM_RELOAD, 3.0s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
			if (m_pWing)
			{
				m_pWing->RemoveWing();
			}
#endif
			m_iWeaponState &= ~WPNSTATE_CCrossBowEX21_WINGON;
		}
	}

	void GetFireInfo(float& flDamage, float& flFireDistance, float& flFireSpread, int& cShots, duration_t& flCycleTime)
	{
		flFireDistance = 6048;
		flFireSpread = 0.055;
		cShots = 7;
		flCycleTime = 0.25s;
		flDamage = 20;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 112;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 149;
#endif
	}

	void SpecialShoot()
	{
		TraceResult tr;
		Vector vecEnd, vecDir;

		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/CCrossBowEX21-2.wav", VOL_NORM, ATTN_NORM, 0, 94);
#ifndef CLIENT_DLL
		float flTouchDamage = GetDirectDamage();
		float flExplodeDamage = GetExplodeDamage();
		float flDistance = 8192.0;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		vecDir = gpGlobals->v_forward;
		vecEnd = vecSrc + vecDir * flDistance;

		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		if (TheBots != NULL && tr.flFraction != 1.0f) {
			TheBots->OnEvent(EVENT_BULLET_IMPACT, this, (CBaseEntity*)&tr.vecEndPos);
		}

		CBaseEntity* pHitEntity = NULL;

		if (tr.flFraction != 1.0f)
		{
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
			pHitEntity = pEntity;
			if (pEntity->pev->takedamage != DAMAGE_NO)
			{
				ClearMultiDamage();
				pEntity->TraceAttack(m_pPlayer->pev, flTouchDamage, vecDir, &tr, (DMG_BULLET | DMG_NEVERGIB));
				ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (player->m_bIsZombie)
					{
						Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
						vecDir.z = 0;

						ApplyKnockbackData(player, vecDir, { 1500, 1500, 1500, 1500, 1.0f });
						player->pev->velocity.z += 200.0f;
					}
				}
			}
		}

		CBaseEntity* pEntity = NULL;
		vecSrc = tr.vecEndPos;

		Vector vecExpPos = vecSrc - vecDir.Normalize() * 20;

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(vecExpPos.x);
		WRITE_COORD(vecExpPos.y);
		WRITE_COORD(vecExpPos.z);
		WRITE_SHORT(m_iExpIndex);
		WRITE_BYTE(10);
		WRITE_BYTE(255);
		WRITE_BYTE(30);
		MESSAGE_END();

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, 0.0, 1, m_vVecAiming.x * 100, TRUE, FALSE);

#ifndef CLIENT_DLL
		UTIL_TempModel(vecSrc, vecDir.Normalize(), g_vecZero, m_iExpModelIndex, 15, 0, 10, false, 0, 255, kRenderTransAdd, NULL, false, 0, 0, 46, 0);
#endif

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 118.0)) != NULL)
		{
			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity->edict() == m_pPlayer->edict())
				continue;

			if (pEntity->IsDormant())
				continue;

			if (pEntity->Classify() == CLASS_NONE)
				continue;

			if (pHitEntity && pEntity == pHitEntity)
				continue;

			if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
				continue;

			vecEnd = pEntity->BodyTarget(vecSrc);

			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, pev->owner, &tr);

			if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
				continue;

			pEntity->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, flExplodeDamage, DMG_EXPLOSION);

			if (pEntity->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pEntity;

				if (player->m_bIsZombie)
				{
					Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
					vecDir.z = 0;

					ApplyKnockbackData(player, vecDir, { 1500, 1500, 1500, 1500, 1.0f });
					player->pev->velocity.z += 200.0f;
				}
			}
		}
#endif
	}

	void WeaponIdle() override
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);		
#ifndef CLIENT_DLL
			if (m_pWing)
			{
				if (m_pWing->pev->body == 0 && m_pPlayer->pev->button & (IN_FORWARD) && m_pPlayer->pev->velocity.Length2D() > 235)
				{
					CreateWingParticle();
					m_pWing->pev->body = 1;
				}
				else
				{
					RemoveWingParticle();
					m_pWing->pev->body = 0;
				}				
			}			
#endif

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		if (m_iWeaponState & WPNSTATE_CCrossBowEX21_WINGON)
			SendWeaponAnim(ANIM_WINGLOOP, UseDecrement() != FALSE);
		else
			SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
	}

	void StopIdleSound(void)
	{
#ifndef CLIENT_DLL
		int player = m_pPlayer->entindex();

		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
		WRITE_BYTE(79);
		WRITE_SHORT(player);
		WRITE_SHORT(CHAN_ITEM);
		WRITE_STRING("weapons/CCrossBowEX21_loop.wav");
		MESSAGE_END();
#endif
	}

	float GetDirectDamage()
	{
		float flDamage = 110;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 4800;	//9600
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 19800;
#endif
		return flDamage;
	}

	float GetExplodeDamage()
	{
		float flDamage = 30;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 1700;	//3500
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 4500;
#endif
		return flDamage;
	}

};

LINK_ENTITY_TO_CLASS(weapon_CCrossBowEX21, CCrossBowEX21)

}