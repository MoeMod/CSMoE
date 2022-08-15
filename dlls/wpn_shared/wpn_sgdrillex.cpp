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
#include "wpn_sgdrillex.h"

#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#include "player/player_mod_strategy.h"
#endif


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	class CSgDrillExField : public CBaseAnimating
	{
	public:
		void Spawn(void);
		void Precache(void);

		void StartFadeOut(void);
		void FadeOut(void);

	public:
		void EXPORT OnThink(void);

	public:
		static CSgDrillExField* Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam);
	private:

		duration_t GetLifeTime(int iType);
		int m_iType;
		time_point_t m_flAttackInterval;
		time_point_t m_flRemoveTime;
	};


	LINK_ENTITY_TO_CLASS(weapon_sgdrillex, CSgdrillEx)

	void CSgdrillEx::Spawn(void)
	{
		Precache();
		m_iId = WEAPON_SGDRILLEX;
		SET_MODEL(ENT(pev), "models/w_sgdrillex.mdl");
		pev->classname = MAKE_STRING("weapon_sgdrillex");

		m_iDefaultAmmo = SGDRILLEX_DEFAULT_GIVE;
		m_iDefaultAmmo2 = 50;
		m_bSlash = 0;
		m_bCanAttack = true;
		m_bShowSmoke = false;

		InitChargeState();
		FallInit();
	}

	void CSgdrillEx::Precache(void)
	{
		PRECACHE_MODEL("models/v_sgdrillex.mdl");
		PRECACHE_MODEL("models/w_sgdrillex.mdl");
		PRECACHE_MODEL("models/p_sgdrillex.mdl");
		PRECACHE_MODEL("models/p_sgdrill_slash.mdl");

		PRECACHE_MODEL("sprites/muzzleflash253.spr");
		PRECACHE_MODEL("sprites/ef_sgdrillex_slash.spr");
		PRECACHE_MODEL("sprites/ef_sgdrillex_smoke.spr");
		PRECACHE_MODEL("sprites/ef_sgdrillex_lightning.spr");

		m_iShellId = m_iShell = PRECACHE_MODEL("models/shell_sgdrill.mdl");

		PRECACHE_SOUND("weapons/sgdrillex-1.wav");
		PRECACHE_SOUND("weapons/sgdrillex_slash1.wav");
		PRECACHE_SOUND("weapons/sgdrillex_slash2.wav");
		PRECACHE_SOUND("weapons/sgdrillex_charge_slash.wav");
		


		m_usFireSgdrillEx = PRECACHE_EVENT(1, "events/sgdrillex.sc");
	}

	int CSgdrillEx::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "sgdrill_buckshot";
		p->iMaxAmmo1 = 105;
		p->pszAmmo2 = "sgdrillex_energy";
		p->iMaxAmmo2 = 100;
		p->iMaxClip = SGDRILLEX_MAX_CLIP;
		p->iSlot = 0;
		p->iPosition = 12;
		p->iId = m_iId = WEAPON_SGDRILLEX;
		p->iFlags = 0;
		p->iWeight = XM1014_WEIGHT;

		return 1;
	}
	int CSgdrillEx::ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}
	BOOL CSgdrillEx::Deploy(void)
	{
		ResetChargeTimer();
#ifndef CLIENT_DLL
		ItemPostFrame();
#endif

		if (DefaultDeploy("models/v_sgdrillex.mdl", "models/p_sgdrillex.mdl", SGDRILLEX_DRAW, "skull8", UseDecrement() != FALSE))
		{
			m_bCanAttack = true;
			m_bShowSmoke = false;
			ResetSkin();
			return TRUE;
		}

		return FALSE;
	}

	int CSgdrillEx::GetPrimaryAttackDamage() const
	{
		int iDamage = 22 * 2;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			iDamage = 42 * 2;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			iDamage = 42 * 2;
#endif
		return iDamage;
	}

	float CSgdrillEx::GetSecondaryAttackDamage() const
	{
		float flDamage = 115.0f * 1.5f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 6.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 6.0f;
#endif
		return flDamage;
	}
	float CSgdrillEx::GetChargeAttackDamage() const
	{
		float flDamage = 115.0f * 1.5f * 1.5f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 6.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 6.0f;
#endif
		return flDamage;
	}

	void CSgdrillEx::PrimaryAttack(void)
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
		}

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}
		m_bSlash = 0;
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(8, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.03, 0.03, 0.0), 4096, BULLET_PLAYER_BUCKSHOT, 0, GetPrimaryAttackDamage());
#endif
		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireSgdrillEx, 0, g_vecZero, g_vecZero, m_vVecAiming.x, m_vVecAiming.y, 7, m_vVecAiming.x * 100, m_iClip != 0, TRUE);

		ResetChargeTimer();
		ResetSkin();

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.32s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.32s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;

		if (m_pPlayer->pev->velocity.Length2D() > 0)
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 1, 3.0, 4.0);
		else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 5.0, 6.0);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 1.5, 2.0);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2.0, 3.0);

		m_bCanAttack = true;
	}

	void CSgdrillEx::Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(SGDRILLEX_MAX_CLIP, SGDRILLEX_RELOAD, 2.9s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_bCanAttack = true;
			ResetSkin();
		}
		m_flTimeNextCharge = invalid_time_point;
	}

	void CSgdrillEx::SecondaryAttack(void)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK2);
		if(m_iSpecialAmmo< 75)
			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireSgdrillEx, 0, g_vecZero, g_vecZero, 0.0, 0.0, 1, 0, 1, 0);

#endif
		SendWeaponAnim(SGDRILLEX_SLASH, UseDecrement() != FALSE);
		SetThink(&CSgdrillEx::DelaySecondaryAttack);
		pev->nextthink = gpGlobals->time + 0.6s;
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sgdrillex_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.3s;
		m_flNextResetModel = gpGlobals->time + 1.2s;

		ResetChargeTimer();

	}

	void CSgdrillEx::DelaySecondaryAttack()
	{
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
		//PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireSgdrillEx, 0, g_vecZero, g_vecZero, 0.0, 0.0, 1, 0, 1, 0);
		KnifeAttack(FALSE,vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 155, 120, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev);
#endif
		
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1s;
		SetThink(nullptr);
	}
	void CSgdrillEx::ResetSkin()
	{
		int m_iSecondaryAmmo = m_iSpecialAmmo;
		int m_iSkinToSet = 0;
		if (m_iSecondaryAmmo && m_iSecondaryAmmo < 15)
		{
			m_iSkinToSet = 0;
		}
		else if (m_iSecondaryAmmo >= 20 && m_iSecondaryAmmo < 40)
		{
			m_iSkinToSet = 1;
		}
		else if (m_iSecondaryAmmo >= 40 && m_iSecondaryAmmo < 60)
		{
			m_iSkinToSet = 2;
		}
		else if (m_iSecondaryAmmo >= 60 && m_iSecondaryAmmo < 75)
		{
			m_iSkinToSet = 3;
		}
		else if (m_iSecondaryAmmo >= 75)
		{
			m_iSkinToSet = 4;
		}
		pev->skin = m_iSkinToSet;
	}
	void CSgdrillEx::ItemPostFrame()
	{
		if (m_flNextResetModel <= gpGlobals->time)
		{
			//m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_sgdrill.mdl");
		}

		bool m_bCanChargeAttack = m_iSpecialAmmo >= 75 ? true : false;
		int m_iButton = m_pPlayer->pev->button;

		if (m_bCanChargeAttack)
		{
			if (m_iButton & IN_ATTACK && m_iButton & IN_ATTACK2 && m_bCanAttack)
			{
				//Similar to secondary
#ifndef CLIENT_DLL
				m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
				SendWeaponAnim(SGDRILLEX_CHARGESLASH, UseDecrement() != FALSE);

				SetThink(&CSgdrillEx::DelayChargeAttack);
				pev->nextthink = gpGlobals->time + 0.6s;

#ifndef CLIENT_DLL
				KnifeAttack(TRUE, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, GetChargeAttackDamage() / 2.0f, 360, 360, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev);
#endif

				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/sgdrillex_charge_slash.wav", VOL_NORM, ATTN_NORM, 0, 94);
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.3s;
				m_flNextResetModel = gpGlobals->time + 1.2s;

				ResetSkin();
				m_flTimeNextCharge = invalid_time_point;
				m_bCanAttack = false;

#ifndef CLIENT_DLL
				CSgDrillExField::Create(0, m_pPlayer->pev->origin, g_vecZero, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);
#endif

			}
		}


		if (m_iSpecialAmmo == 100)
			return CBasePlayerWeapon::ItemPostFrame();

		if (m_fInReload > 0)
			return CBasePlayerWeapon::ItemPostFrame();
		if(m_flTimeNextCharge == invalid_time_point)
			return CBasePlayerWeapon::ItemPostFrame();

		if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
		{
			m_iSpecialAmmo = m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]  += int((gpGlobals->time - m_flTimeNextCharge) / 0.8s + 5.0);
			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] % 5 != 0)
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]  += m_iSpecialAmmo % 5 - 1;
			}


			if (m_iSpecialAmmo >= 100)
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]  = 100;
				m_iSpecialAmmo = 100;
#ifndef CLIENT_DLL
				if (!m_bShowSmoke)
					PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireSgdrillEx, 0, g_vecZero, g_vecZero, 0.0, 0.0, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 75 ? 1 : 0, 0, FALSE, FALSE);
#endif
				ResetSkin();
			}

			m_flTimeNextCharge = gpGlobals->time + 0.8s;

			if (m_iSpecialAmmo == 75)
			{
				ResetSkin();
#ifndef CLIENT_DLL
				if (!m_bShowSmoke)
					PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireSgdrillEx, 0, g_vecZero, g_vecZero, 0.0, 0.0, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 75 ? 1 : 0, 0, FALSE, FALSE);
#endif
				m_flTimeNextCharge = invalid_time_point;
			}
			ResetSkin();
				
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}
	void CSgdrillEx::DelayChargeAttack()
	{
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

#ifndef CLIENT_DLL
		KnifeAttack(TRUE,vecSrc, gpGlobals->v_forward, GetChargeAttackDamage(), 360, 180, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev);
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1s;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK2);

		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireSgdrillEx, 0, g_vecZero, g_vecZero, 0.0, 0.0, 1, 0, 1, 0);


		CSgDrillExField::Create(0, m_pPlayer->pev->origin, g_vecZero, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);
		CSgDrillExField::Create(1, m_pPlayer->pev->origin, g_vecZero, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);
#endif
		//Update weapon data and reset the skin.
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] -= 75;
		m_iSpecialAmmo -= 75;
		ResetSkin();
		PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireSgdrillEx, 0, g_vecZero, g_vecZero, 0.0, 0.0, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 75 ? 1 : 0, 0, FALSE, FALSE);

		m_bCanAttack = true;
		m_bShowSmoke = false;

		SetThink(nullptr);
	}

	void CSgdrillEx::AddKickRate(CBasePlayer* pPlayer, Vector vecSrc, float amount, float boost)
	{
		//from AE
		Vector vecDir = (pPlayer->pev->origin - vecSrc).Normalize();

		pPlayer->pev->velocity.x += vecDir.x * amount;
		pPlayer->pev->velocity.y += vecDir.y * amount;
		pPlayer->pev->velocity.z += boost;
	}

	void CSgdrillEx::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

#ifndef CLIENT_DLL
		if(!m_bShowSmoke)
			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireSgdrillEx, 0, g_vecZero, g_vecZero, 0.0, 0.0, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 75 ? 1 : 0, 0, FALSE, FALSE);
#endif

		ResetChargeTimer();
		ResetSkin();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(SGDRILLEX_IDLE, UseDecrement() != FALSE);

	}
#ifndef CLIENT_DLL
	BOOL CSgdrillEx::KnifeAttack(bool IsCharged, Vector vecSrc, Vector vecDir, float flDamage, float flRadius, float flAngleDegrees, int bitsDamageType,
		entvars_t* pevInflictor, entvars_t* pevAttacker)
	{
		TraceResult tr;
		BOOL result = 0;

		vecSrc.z += 1;

		if (!pevAttacker)
			pevAttacker = pevInflictor;

		Vector vecEnd = vecSrc + vecDir.Normalize() * flRadius;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pevAttacker), &tr);

		if (tr.flFraction >= 1) {
			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(pevAttacker), &tr);

			if (tr.flFraction < 1) {
				CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

				if (!pHit || pHit->IsBSPModel()) {
					FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(pevAttacker));
				}

				vecEnd = tr.vecEndPos;
			}
		}

		if (tr.flFraction < 1) {
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (pHit && pHit->IsBSPModel() && pHit->pev->takedamage != DAMAGE_NO) {
				const float flAdjustedDamage = flDamage;
				ClearMultiDamage();
				pHit->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
				ApplyMultiDamage(pevInflictor, pevAttacker);
			}

			float flVol = 1;
			BOOL fHitWorld = TRUE;
			if (pHit && pHit->Classify() != CLASS_NONE && pHit->Classify() != CLASS_MACHINE) {
				flVol = 0.1f;
				fHitWorld = FALSE;
			}

			if (fHitWorld) {
				TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
				result = TRUE;
			}
		}

		CBaseEntity* pEntity = nullptr;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != nullptr) {
			if (pEntity->pev->takedamage != DAMAGE_NO) {
				if (pEntity->IsBSPModel())
					continue;

				if (pEntity->pev == pevAttacker)
					continue;

				Vector vecSpot = pEntity->BodyTarget(vecSrc);
				vecSpot.z = vecEnd.z;
				UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

				if (AngleBetweenVectors(tr.vecEndPos - vecSrc, vecDir) > flAngleDegrees)
					continue;

				if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict()) {
					if (tr.fStartSolid) {
						tr.vecEndPos = vecSrc;
						tr.flFraction = 0;
					}

					if (tr.flFraction == 1.0f) {
						pEntity->TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
					}

					Vector vecRealDir = (tr.vecEndPos - vecSrc).Normalize();
					m_bSlash = 1;

					ClearMultiDamage();
					pEntity->TraceAttack(pevInflictor, flDamage, vecRealDir, &tr, bitsDamageType);
					ApplyMultiDamage(pevInflictor, pevAttacker);

					if (!pEntity->IsPlayer())
						continue;

					CBasePlayer* pVictim = dynamic_cast<CBasePlayer*>(pEntity);
					if (!IsCharged)
					{
						if (pVictim->m_bIsZombie && !(pVictim->m_iBuff & BUFF_IGNORE_KNOCKBACK)) // Zombie Knockback...
						{
							AddKickRate(pVictim, m_pPlayer->pev->origin, 1350, 120);
							auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(pVictim->m_pModStrategy.get());
							if (strategy)
							{
								strategy->SetStunSpeedTime(1.0s, 150.0F);
								strategy->SetStunGravityTime(1.0s);
							}
							pVictim->m_flVelocityModifier = 1.0;
						}

					}
					else
					{
						if (pVictim->m_bIsZombie && !(pVictim->m_iBuff & BUFF_IGNORE_KNOCKBACK)) // Zombie Knockback...
						{
							AddKickRate(pVictim, m_pPlayer->pev->origin, 1500, 120);

							MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pVictim->pev->origin);
							WRITE_BYTE(TE_EXPLOSION);
							WRITE_COORD(pVictim->pev->origin.x);
							WRITE_COORD(pVictim->pev->origin.y);
							WRITE_COORD(pVictim->pev->origin.z);
							WRITE_SHORT(MODEL_INDEX("sprites/ef_sgdrillex_lightning.spr"));
							WRITE_BYTE(10);
							WRITE_BYTE(30);
							WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
							MESSAGE_END();

							auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(pVictim->m_pModStrategy.get());
							if (strategy)
							{
								strategy->SetStunSpeedTime(2.0s, 80.0F);
								strategy->SetStunGravityTime(2.0s);
							}


						}
					}
					
					result = TRUE;

				}
			}
		}

		return result;
	}
#endif


	void CSgdrillEx::FindHullIntersection(const Vector& vecSrc, TraceResult& tr, const vec3_t pflMins, const vec3_t pfkMaxs, edict_t* pEntity)
	{
		TraceResult trTemp;
		float flDistance = 1000000;
		const vec3_t pflMinMaxs[] = { pflMins, pfkMaxs };
		Vector vecHullEnd = tr.vecEndPos;

		vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
		TRACE_LINE(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &trTemp);

		if (trTemp.flFraction < 1)
		{
			tr = trTemp;
			return;
		}

		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				for (int k = 0; k < 2; k++)
				{
					Vector vecEnd;
					vecEnd.x = vecHullEnd.x + pflMinMaxs[i][0];
					vecEnd.y = vecHullEnd.y + pflMinMaxs[j][1];
					vecEnd.z = vecHullEnd.z + pflMinMaxs[k][2];

					TRACE_LINE(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &trTemp);

					if (trTemp.flFraction < 1)
					{
						float flThisDistance = (trTemp.vecEndPos - vecSrc).Length();

						if (flThisDistance < flDistance)
						{
							tr = trTemp;
							flDistance = flThisDistance;
						}
					}
				}
			}
		}
	}


	void CSgdrillEx::Holster(int skiplocal)
	{
		ResetChargeTimer();
		SetThink(nullptr);
		return CBasePlayerWeapon::Holster(skiplocal);
	}
	void CSgdrillEx::InitChargeState(void)
	{
		m_iSpecialAmmo = 50;
		m_flTimeNextCharge = invalid_time_point;

		pev->skin = 0;
	}
	void CSgdrillEx::ResetChargeTimer(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 100 && m_flTimeNextCharge == invalid_time_point)
			m_flTimeNextCharge = gpGlobals->time + 1.0s;
	}


#ifndef CLIENT_DLL
	LINK_ENTITY_TO_CLASS(sgdrillexfield, CSgDrillExField)
	CSgDrillExField* CSgDrillExField::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{

		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("sgdrillexfield"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CSgDrillExField* pDragon = (CSgDrillExField*)Instance(pent);

		if (pDragon)
		{
			pDragon->m_iType = iType;
			pDragon->pev->owner = pentOwner;

			pDragon->pev->origin = vecOrigin;
			if (iType == 2)
			{
				pDragon->pev->angles = g_vecZero;
				pDragon->pev->angles.y = -vecAngles.y;
			}
			else
				pDragon->pev->angles = vecAngles;


			pDragon->m_iTeam = iTeam;
			pDragon->Spawn();


		}

		return pDragon;
	}

	void CSgDrillExField::Spawn(void)
	{
		Precache();
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_FLY;

		pev->frame = 0;
		pev->sequence = 0;
		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 255;

		ResetSequenceInfo();


		if (!m_iType)
		{
			SET_MODEL(ENT(pev), "models/ef_sgdrillex_slash.mdl");
			//UTIL_SetOrigin(pev, pev->origin + Vector(0, 0, -10));
			pev->scale *= 1.2;
			

		}
		else if(m_iType == 1)
		{
			

			SET_MODEL(ENT(pev), "models/ef_sgdrillex_ladar.mdl");
			//UTIL_SetOrigin(pev, pev->origin + Vector(0, 0, -10));
			
		}
		else
		{
			SET_MODEL(ENT(pev), "models/ef_sbmine_lightning.mdl");
			//UTIL_SetOrigin(pev, pev->origin + Vector(0, 0, -10));
			
		}
		m_flRemoveTime = gpGlobals->time + GetLifeTime(m_iType);

		SetThink(&CSgDrillExField::OnThink);
		pev->nextthink = gpGlobals->time + 0.01s;

	}
	void CSgDrillExField::Precache(void)
	{
		PRECACHE_MODEL("models/ef_sgdrillex_slash.mdl");	//ground
		PRECACHE_MODEL("models/ef_sgdrillex_ladar.mdl");	//ground
		PRECACHE_MODEL("models/ef_sbmine_lightning.mdl");	//ground

	}
	void CSgDrillExField::OnThink()
	{
		pev->nextthink = gpGlobals->time + 0.01s;
		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);


		if (pAttackPlayer)
		{
			UTIL_SetOrigin(pev, pAttackPlayer->pev->origin + Vector(0, 0, -10));

		}
		if (m_flRemoveTime < gpGlobals->time)
		{
			pev->nextthink = gpGlobals->time + 0.01s;
			SetThink(&CSgDrillExField::StartFadeOut);
			m_flRemoveTime = invalid_time_point;
			return;
		}
	}
	void CSgDrillExField::StartFadeOut()
	{
		if (pev->rendermode != kRenderTransTexture)
		{
			pev->renderamt = 255.0f;
			pev->rendermode = kRenderTransTexture;
		}

		pev->solid = SOLID_NOT;
		pev->avelocity = g_vecZero;
		pev->nextthink = gpGlobals->time + 0.001s;

		SetThink(&CSgDrillExField::FadeOut);
	}

	void CSgDrillExField::FadeOut()
	{
		if (pev->renderamt > 50)
		{
			pev->renderamt -= 50.0f;
			pev->nextthink = gpGlobals->time + 0.001s;
		}
		else
		{
			pev->renderamt = 0.0f;
			pev->nextthink = gpGlobals->time + 0.001s;
			SetThink(&CSgDrillExField::SUB_Remove);
		}
	}
	duration_t CSgDrillExField::GetLifeTime(int m_iType)
	{

		return !m_iType? 1.43s:3.03s;
	}
#endif

}
