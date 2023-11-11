/*
wpn_linkattacksg.cpp - Port of Zombie IV Mod
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

	class CLinkAttackSG : public LinkWeaponTemplate< CLinkAttackSG,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_XM1014>::template type,
		TPrecacheEvent,
		TDeployDefault
	>
	{
	public:
		static constexpr const char* V_Model = "models/v_linkattacksg.mdl";
		static constexpr const char* P_Model = "models/p_linkattacksg.mdl";
		static constexpr const char* W_Model = "models/w_linkattacksg.mdl";
		static constexpr const char* EventFile = "events/linkattacksg.sc";
		static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
		static constexpr const char* ClassName = "weapon_linkattacksg";
		static constexpr const char* AnimExtension = "shotgun";
		static constexpr int MaxClip = 30;
		static constexpr int MaxSecondClip = 3;
		static constexpr float Spread = 0.03;

		static constexpr int NumShot = 10;
		static constexpr float Distance = 8000.0;
		static constexpr duration_t CycleTime = 0.25s;
		static constexpr duration_t DeployAttackTime = 0.45s;
		static constexpr duration_t ReloadFirst = 0.45s;
		static constexpr duration_t ReloadAfter = 0.35s;

		static constexpr duration_t ProjCycleTime = 0.25s;
		static constexpr float ProjSpeed = 2000.0;

		enum
		{
			ANIM_IDLE1,
			ANIM_SHOOT1,
			ANIM_SHOOT2,
			ANIM_RELOAD_INSERT,
			ANIM_RELOAD_AFTER,
			ANIM_RELOAD_START,
			ANIM_DRAW,
			ANIM_SHOOTB,
		};

		enum part_e
		{
			PART_IDLE = 0,
			PART_CHANGE_START,
			PART_CHANGE,
			PART_CHANGE_END,
		};

		enum linkattacksg_bodygroup_e
		{
			GROUP_WEAPON1,
			GROUP_WEAPON2,
			GROUP_SPHERE,
			GROUP_HANDS,
		};

		enum linksphere_effect_state_e
		{
			EFFECT_NONE,
			EFFECT_FADEIN,
			EFFECT_FADEOUT
		};


		static constexpr float PlayerMaxSpeed = 260;
		static constexpr auto BulletType = BULLET_PLAYER_BUCKSHOT;
		static constexpr int Penetration = 1;


		KnockbackData KnockBack = { 800.0f, 550.0f, 700.0f, 550.0f, 0.3f };

		void Spawn() override;
		void Precache() override;
		int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
		void SetSubModel();
		void SecondaryAttack() override;
		void PrimaryAttack() override;
		BOOL Deploy() override;
		void Reload() override;
		void WeaponIdle(void) override;
		void ItemPostFrame() override;
		void Holster(int skiplocal = 0) override;
		void SendWeaponAnim2(int iAnim);
	
		duration_t m_flPumpTime;
		duration_t m_flTimeWeaponIdle2;
		time_point_t m_flTimeNextCharge;
		int m_iDefaultAmmo2;
		int m_iModelIndex;
		std::vector<int> m_TargetVector;
		EHANDLE m_hLinkSphere[MaxSecondClip + 1];

#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_linkattacksg" , 100 }; }
#endif

		WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 0 }; }

		int GetItemInfo(ItemInfo* p) override
		{
			p->pszName = STRING(pev->classname);
			p->pszAmmo1 = "buckshot_linkattacksg";
			p->iMaxAmmo1 = 60;
			p->pszAmmo2 = "linkattacksg_b";
			p->iMaxAmmo2 = -1;
			p->iMaxClip = 30;
			p->iSlot = 0;
			p->iPosition = 5;
			p->iId = m_iId = WEAPON_LINKATTACKSG;
			p->iFlags = 0;
			p->iWeight = XM1014_WEIGHT;

			return 1;
		}


		float GetArmorRatioModifier() override
		{
			return 1.3;
		}

		KnockbackData GetKnockBackData() override
		{
			return { 800.0f, 550.0f, 700.0f, 550.0f, 0.3f };
		}

		duration_t GetChargeTime()
		{
			duration_t flTime = 30.0s;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flTime = 8.0s;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flTime = 8.0s;
#endif
			return flTime;
		}


		int GetDamage()
		{
			int flDamage = 12;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 74;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 140;
#endif
			return flDamage;
		}

		float GetSphereExplodeDamage() const
		{
			float flDamage = 57.0;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 2308.0;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 5681.0;
#endif
			return flDamage;
		}

		float GetSphereLinkAttackDamage() const
		{
			float flDamage = 57.0;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 2308.0;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 5681.0;
#endif
			return flDamage;
		}
	};

	LINK_ENTITY_TO_CLASS(weapon_linkattacksg, CLinkAttackSG)


#ifndef CLIENT_DLL

#include "gamemode/mod_zb1.h"

	class CLinkAttackSGProj : public CBaseEntity
	{
	public:
		static constexpr duration_t SphereDuration = 3.0s;
		static constexpr float SphereRadius = 5.0;
		static constexpr float SphereMaxRenderAmout = 128.0;
		static constexpr duration_t SphereExplodeFixedMoveTime = 0.1s;
		static constexpr float SphereExplodeFixedMoveSpeed = 100.0;
		static constexpr duration_t SphereFixedMoveTime = 0.3s;
		static constexpr float SphereFixedMoveSpeed = 150.0;

		enum linksphere_effect_state_e
		{
			EFFECT_NONE,
			EFFECT_FADEIN,
			EFFECT_FADEOUT
		};


		float GetArmorRatioModifier() { return 1.3; }
		int GetWeaponsId(void) { return WEAPON_LINKATTACKSG; }
		void Spawn() override
		{
			Precache();

			SetThink(&CLinkAttackSGProj::FollowThink);
			SetTouch(&CLinkAttackSGProj::OnTouch);
			SET_MODEL(this->edict(), "sprites/ef_linkattacksg_projectile.spr");

			pev->rendermode = kRenderTransAdd;
			pev->renderfx = kRenderFxNone;
			pev->renderamt = 200;
			pev->framerate = 10;
			pev->scale = 0.1;
			
			pev->solid = SOLID_BBOX;
			pev->movetype = MOVETYPE_FLYMISSILE;
			pev->nextthink = gpGlobals->time + 0.1s;

			m_flFadeInTime = 0.5s;
			m_flFadeOutTime = 0.5s;

			m_flMaxFrames = float(MODEL_FRAMES(pev->modelindex) - 1);

			UTIL_SetOrigin(pev, pev->origin);
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
		}

		void Precache() override
		{
			PRECACHE_MODEL("sprites/ef_linkattacksg_projectile.spr");
			PRECACHE_MODEL("models/ef_linkattacksg_field.mdl");

			PRECACHE_SOUND("weapons/linkattacksg-2_2.wav");

			m_iExpIndex = PRECACHE_MODEL("sprites/ef_linkattacksg_exp.spr");
		}

		void EXPORT OnTouch(CBaseEntity* pOther)
		{
			if (pOther == m_pOwner)
				return;
	
			m_flRemoveTime = gpGlobals->time + SphereDuration + m_flFadeInTime + m_flFadeOutTime;
			m_iEffectState = EFFECT_FADEIN;
			m_flEffectTime = gpGlobals->time;
			SetThink(&CLinkAttackSGProj::LinkSphereThink);

			SET_MODEL(this->edict(), "models/ef_linkattacksg_field.mdl");
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_NONE;

			pev->skin = 0;

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iExpIndex);
			WRITE_BYTE(15);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();

			EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/linkattacksg-2_2.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

			Explode(true);
		}

		void Explode(bool isExp)
		{
			TraceResult tr;
			CBaseEntity* pEntity = NULL;

			Vector vecEnd;

			float flDamage = isExp ? m_flSphereExplodeDamage : m_flSphereLinkAttackDamage;

			float flRadius = SphereRadius * 39.37;
			while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
			{
				if (pEntity->edict() == pev->owner)
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
					continue;

				if (std::find(m_pWeapon->m_TargetVector.begin(), m_pWeapon->m_TargetVector.end(), pEntity->entindex()) != m_pWeapon->m_TargetVector.end())
					continue;

				m_pWeapon->m_TargetVector.push_back(pEntity->entindex());

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

				if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
					continue;

				int falloff = flDamage / flRadius;

				float flCurrentDamage = max(0, flDamage - (pev->origin - pEntity->pev->origin).Length() * falloff);

				pEntity->m_LastHitGroup = HITGROUP_CHEST;
				pEntity->TakeDamage(pev, m_pOwner->pev, flCurrentDamage, DMG_NEVERGIB | DMG_BULLET);

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (player->m_bIsZombie)
					{
						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							if(isExp)
								strategy->SetStunSpeedTime(SphereExplodeFixedMoveTime, SphereExplodeFixedMoveSpeed);
							else
								strategy->SetStunSpeedTime(SphereFixedMoveTime, SphereFixedMoveSpeed);
						}
					}
				}
			}
		}

		void EXPORT FollowThink()
		{
			pev->frame += pev->framerate * gpGlobals->frametime / 1s;

			if (pev->frame > m_flMaxFrames)
				pev->frame = fmod(pev->frame, m_flMaxFrames);

			pev->nextthink = gpGlobals->time + 0.01s;

			if (!IsInWorld())
			{
				UTIL_Remove(this);
				return;
			}
		}

		void EXPORT LinkSphereThink()
		{
			pev->nextthink = gpGlobals->time + 0.2s;

			pev->skin = 1;

			if(!m_pWeapon)
			{
				RemoveLinkSphere();
			}

			if (m_iEffectState != EFFECT_FADEOUT && m_pOwner && (!m_pOwner->IsAlive() || m_pOwner->m_bIsZombie))
			{
				RemoveLinkSphere();
			}

			if (m_flRemoveTime != invalid_time_point && m_flRemoveTime < gpGlobals->time)
			{
				RemoveLinkSphere();
			}

			if (m_iEffectState == EFFECT_FADEIN)
				EF_Appear();

			if (m_iEffectState == EFFECT_FADEOUT)
				EF_Hide();
		}

		void RemoveLinkSphere(void)
		{
			m_flRemoveTime = invalid_time_point;
			m_flEffectTime = gpGlobals->time;
			m_iEffectState = EFFECT_FADEOUT;
		}

		void EF_Appear(void)
		{
			pev->nextthink = gpGlobals->time + 0.1s;

			duration_t interval = gpGlobals->time - m_flEffectTime;

			if (interval > m_flFadeInTime)
			{
				m_flEffectTime = invalid_time_point;
				m_iEffectState = EFFECT_NONE;

				pev->rendermode = kRenderNormal;
				pev->renderfx = kRenderFxNone;
				pev->renderamt = SphereMaxRenderAmout;

				pev->skin = 1;

			}
			else
			{
				pev->rendermode = kRenderTransAlpha;
				pev->renderamt = (interval / m_flFadeInTime) * SphereMaxRenderAmout;
			}
		}

		void EF_Hide(void)
		{
			pev->nextthink = gpGlobals->time + 0.1s;

			duration_t interval = gpGlobals->time - m_flEffectTime;

			if (interval > m_flFadeOutTime)
			{
				m_flEffectTime = invalid_time_point;
				m_iEffectState = EFFECT_NONE;

				pev->nextthink = invalid_time_point;
				UTIL_Remove(this);
			}
			else
			{
				pev->rendermode = kRenderTransAlpha;
				pev->renderamt = (1.0 - interval / m_flFadeOutTime) * SphereMaxRenderAmout;
			}
		}

		void Init(Vector vecVelocity, float flTouchDamage, float flExplodeDamage, int iTeam, CBasePlayer * pOwner, CLinkAttackSG * pWeapon)
		{
			std::tie(m_flSphereExplodeDamage, m_flSphereLinkAttackDamage, m_iTeam, m_pOwner, m_pWeapon) = std::make_tuple(flTouchDamage, flExplodeDamage, iTeam, pOwner, pWeapon);
			m_vecStartVelocity = pev->velocity = std::move(vecVelocity);
		}

		KnockbackData GetKnockBackData() override { return  { 500.0f, 450.0f, 450.0f, 320.0f, 0.35f }; }

		time_point_t m_flEffectTime;
		time_point_t m_flRemoveTime;
		duration_t m_flFadeInTime;
		duration_t m_flFadeOutTime;
		float m_flMaxFrames;
		float m_flSphereExplodeDamage;
		float m_flSphereLinkAttackDamage;
		int m_iEffectState;
		int m_iExpIndex;
		Vector m_vecStartVelocity;
		CBasePlayer* m_pOwner;
		CLinkAttackSG* m_pWeapon;

	protected:
		void Remove()
		{
			SetThink(nullptr);
			SetTouch(nullptr);
			pev->effects |= EF_NODRAW; // 0x80u
			return UTIL_Remove(this);
		}
	};
	LINK_ENTITY_TO_CLASS(linkattacksg_proj, CLinkAttackSGProj)
#endif

	void CLinkAttackSG::Spawn()
	{
		m_flTimeWeaponIdle2 = 0s;
		m_iDefaultAmmo2 = 0;
		Base::Spawn();
	}

	void CLinkAttackSG::Precache()
	{
		m_iModelIndex = PRECACHE_MODEL(V_Model);
		PRECACHE_MODEL("sprites/muzzleflash346.spr");
		PRECACHE_MODEL("sprites/muzzleflash347.spr");

		PRECACHE_MODEL("sprites/ef_linkattacksg_laser01.spr");
		PRECACHE_MODEL("sprites/ef_linkattacksg_laser02.spr");
		PRECACHE_MODEL("sprites/ef_linkattacksg_oil01.spr");
		PRECACHE_MODEL("sprites/ef_linkattacksg_oil02.spr");
		PRECACHE_MODEL("sprites/ef_linkattacksg_sphere_v.spr");
		PRECACHE_MODEL("sprites/ef_linkattacksg_sphere01.spr");
		PRECACHE_MODEL("sprites/ef_linkattacksg_sphere02.spr");
		PRECACHE_MODEL("sprites/ef_linkattacksg_sphere03.spr");

		PRECACHE_SOUND("weapons/linkattacksg_change_start.wav");
		PRECACHE_SOUND("weapons/linkattacksg_change_end.wav");

		return Base::Precache();
	}

	

	int CLinkAttackSG::ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo2)
		{
			m_iWeaponState = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	void CLinkAttackSG::SetSubModel()
	{
		pev->modelindex = m_iModelIndex;

		int iSubModel;

		if (m_iWeaponState > 0)
		{
			pev->skin = 1;
			iSubModel = 1;
		}
		else
		{
			pev->skin = 0;
			iSubModel = 0;
		}

		if (GetBodygroup(GROUP_SPHERE) != iSubModel)
		{
			SetBodygroup(GROUP_SPHERE, iSubModel);
		}

		int hands = 0;

		if (m_pPlayer && m_pPlayer->m_bIsFemale)
			hands = 1;

		SetBodygroup(GROUP_HANDS, hands);

		pev->modelindex = 0;
	}

	void CLinkAttackSG::SecondaryAttack(void)
	{
		if (m_iWeaponState <= 0)
			return;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		if (m_iWeaponState == MaxSecondClip)
			m_flTimeNextCharge = gpGlobals->time + GetChargeTime();

		m_iWeaponState--;
	
		if (!m_iWeaponState)
		{
			SetSubModel();
			SendWeaponAnim2(PART_CHANGE_END);
			m_flTimeWeaponIdle2 = UTIL_WeaponTimeBase() + 0.7s;

			EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/linkattacksg_change_end.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
		}

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

#ifndef CLIENT_DLL
		int delta = Q_min(MaxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

		// Add them to the clip
		m_iClip += delta;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= delta;
#endif
		m_fInSpecialReload = 0;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + ProjCycleTime;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + ProjCycleTime;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, TRUE, FALSE);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
		int index;

		for (index = 0; index <= MaxSecondClip; index++)
		{
			if (!m_hLinkSphere[index].Get())
				break;
		}

		if (index == -1 || index >= MaxSecondClip)
			return;

		Vector vecProjDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

		CLinkAttackSGProj* pEnt = static_cast<CLinkAttackSGProj*>(CBaseEntity::Create("linkattacksg_proj", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
		if (pEnt)
		{
			m_hLinkSphere[index] = pEnt;

			pEnt->Init(vecProjDir * ProjSpeed, GetSphereExplodeDamage(), GetSphereLinkAttackDamage(), m_pPlayer->m_iTeam, m_pPlayer, this);
		}
#endif


	}

	BOOL CLinkAttackSG::Deploy(void)
	{
		if (m_iWeaponState < MaxSecondClip)
		{
			if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
			{
				m_iWeaponState += int((gpGlobals->time - m_flTimeNextCharge) / GetChargeTime());
				m_flTimeNextCharge = gpGlobals->time + GetChargeTime();
				if (m_iWeaponState >= MaxSecondClip)
					m_iWeaponState = MaxSecondClip;
			}

			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;
		}

		if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, AnimExtension, UseDecrement() != FALSE, DeployAttackTime))
		{
			SetSubModel();
			SendWeaponAnim2(m_iWeaponState > 0 ? PART_CHANGE : PART_IDLE);
			return TRUE;
		}
		return FALSE;
	}

	void CLinkAttackSG::PrimaryAttack(void)
	{
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
		m_pPlayer->FireBullets(NumShot, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(Spread, Spread, 0), Distance, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0, Spread, 0, 0, FALSE, NumShot);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

		m_TargetVector.clear();

		for (int i = 0; i <= MaxSecondClip; i++)
		{
			if (m_hLinkSphere[i].Get() != NULL)
			{
				if (((CLinkAttackSGProj*)((CBaseEntity*)m_hLinkSphere[i]))->m_iEffectState == EFFECT_NONE)
				{
					((CLinkAttackSGProj*)((CBaseEntity*)m_hLinkSphere[i]))->Explode(false);
					((CLinkAttackSGProj*)((CBaseEntity*)m_hLinkSphere[i]))->pev->skin = 2;
				}			
			}
		}

		m_TargetVector.clear();
#endif
		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.2s;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + CycleTime;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + CycleTime;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

		m_fInSpecialReload = 0;

		if (m_pPlayer->pev->flags & FL_ONGROUND)
			m_pPlayer->pev->punchangle.x -= 3.0;
		else
			m_pPlayer->pev->punchangle.x -= 5.0;
	}

	void CLinkAttackSG::Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == MaxClip)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (!m_fInSpecialReload)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			SendWeaponAnim(ANIM_RELOAD_START, UseDecrement() != FALSE);

			m_fInSpecialReload = 1;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + ReloadFirst;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ReloadFirst;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + ReloadFirst;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + ReloadFirst;
		}
		else if (m_fInSpecialReload == 1)
		{
			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_fInSpecialReload = 2;
			SendWeaponAnim(ANIM_RELOAD_INSERT, UseDecrement() != FALSE);

			m_flNextReload = UTIL_WeaponTimeBase() + ReloadAfter;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ReloadAfter;
		}
		else
		{
			m_iClip++;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			m_fInSpecialReload = 1;
		}
	}

	void CLinkAttackSG::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
			m_flPumpTime = zero_duration;

		m_flTimeWeaponIdle2 -= 0.01s;

		if (m_flTimeWeaponIdle2 < UTIL_WeaponTimeBase())
		{
			SendWeaponAnim2(m_iWeaponState > 0 ? PART_CHANGE : PART_IDLE);

			m_flTimeWeaponIdle2 = UTIL_WeaponTimeBase() + 20s;
		}

		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
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
					SendWeaponAnim(ANIM_RELOAD_AFTER, UseDecrement() != FALSE);

					m_fInSpecialReload = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				}
			}
			else
			{
				SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
			}
					
		}
	}

	void CLinkAttackSG::ItemPostFrame()
	{
		if (m_iWeaponState < MaxSecondClip)
		{
			if (m_flTimeNextCharge <= gpGlobals->time)
			{
				m_iWeaponState++;

				if (m_iWeaponState == 1)
				{
					SetSubModel();
					SendWeaponAnim2(PART_CHANGE_START);
					m_flTimeWeaponIdle2 = UTIL_WeaponTimeBase() + 0.7s;

					EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "weapons/linkattacksg_change_start.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);
				}

				if (m_iWeaponState >= MaxSecondClip)
					m_iWeaponState = MaxSecondClip;

				m_flTimeNextCharge = gpGlobals->time + GetChargeTime();

				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;
			}

		}

		return CBasePlayerWeapon::ItemPostFrame();
	}

	void CLinkAttackSG::Holster(int skiplocal)
	{
		m_flTimeNextCharge = gpGlobals->time;

		CBasePlayerWeapon::Holster(skiplocal);
	}

	void CLinkAttackSG::SendWeaponAnim2(int iAnim)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(50);
		WRITE_SHORT(WEAPON_LINKATTACKSG);
		WRITE_BYTE(iAnim);
		WRITE_BYTE(pev->skin);
		MESSAGE_END();
#endif
	}

}