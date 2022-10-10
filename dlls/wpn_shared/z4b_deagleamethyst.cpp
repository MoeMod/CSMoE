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
#include "z4b_deagleamethyst.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL //if define CLIENT = define CLIENT  ; if not define CLIENT = define SERVER
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(z4b_deagleamethyst, CDEAGLEAMETHYST) //武器注册

		void CDEAGLEAMETHYST::Spawn(void) 
	{
		pev->classname = MAKE_STRING("z4b_deagleamethyst");

		Precache();
		m_iId = WEAPON_Z4B_DEAGLEAMETHYST;
		SET_MODEL(ENT(pev), "models/w_deagle_amethyst.mdl");

		m_iDefaultAmmo = 9999;
		m_flAccuracy = 0.9;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_fMaxSpeed = 250;

		FallInit();
	}

	void CDEAGLEAMETHYST::Precache(void)
	{
		PRECACHE_MODEL("models/v_deagle_amethyst.mdl");
		PRECACHE_MODEL("models/p_deagle_amethyst.mdl");
#ifdef ENABLE_SHIELD
		PRECACHE_MODEL("models/shield/v_shield_deagle.mdl");
#endif
		PRECACHE_MODEL("models/w_deagle_amethyst.mdl");

		PRECACHE_SOUND("weapons/deagle-1.wav");
		PRECACHE_SOUND("weapons/deagle-2.wav");
		PRECACHE_SOUND("weapons/de_clipout.wav");
		PRECACHE_SOUND("weapons/de_clipin.wav");
		PRECACHE_SOUND("weapons/de_deploy.wav");

		m_iShell = PRECACHE_MODEL("models/pshell.mdl");
		m_usFireDEAGLEAMETHYST = PRECACHE_EVENT(1, "events/deagleamethyst.sc");
	}

	int CDEAGLEAMETHYST::GetItemInfo(ItemInfo* p) //武器信息
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "ammo_amethyst";
		p->iMaxAmmo1 = 9999;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = DEAGLE_MAX_CLIP;
		p->iSlot = 1;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_Z4B_DEAGLEAMETHYST;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1; //“读取成功”的意思，就和int main()
	}

	BOOL CDEAGLEAMETHYST::Deploy(void)
	{
		m_flAccuracy = 0.9;
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;
		m_fMaxSpeed = 250;
#ifdef ENABLE_SHIELD
		if (m_pPlayer->HasShield() != false)
			return DefaultDeploy("models/shield/v_shield_deagle.mdl", "models/shield/p_shield_deagle.mdl", DEAGLE_DRAW, "shieldgun", UseDecrement() != FALSE);
		else
#endif
		{
			return DefaultDeploy("models/v_deagle_amethyst.mdl", "models/p_deagle_amethyst.mdl", 2, "onehanded", UseDecrement() != FALSE);
		}
	}

	void CDEAGLEAMETHYST::PrimaryAttack(void)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			DEAGLEAMETHYSTFire((1.5) * (1 - m_flAccuracy), 0.3s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			DEAGLEAMETHYSTFire((0.25) * (1 - m_flAccuracy), 0.3s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			DEAGLEAMETHYSTFire((0.115) * (1 - m_flAccuracy), 0.3s, FALSE);
		else
			DEAGLEAMETHYSTFire((0.13) * (1 - m_flAccuracy), 0.3s, FALSE);

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(0.9, 0.45, 0.35, 0.04, 5.25, 3.5, 4);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.45, 0.3, 0.2, 0.0275, 4.0, 2.25, 7);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.275, 0.2, 0.125, 0.02, 3.0, 1.0, 9);
		else
			KickBack(0.3, 0.225, 0.125, 0.02, 3.25, 1.25, 8);
	}


	void CDEAGLEAMETHYST::DEAGLEAMETHYSTFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)		//弹丸散度   开一枪需要的时间   开启自动瞄准
	{
		flCycleTime -= 0.075s;
		m_iShotsFired++;

		if (m_iShotsFired > 1)
			return;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;

			if (m_flAccuracy > 0.9)
				m_flAccuracy = 0.9;
			else if (m_flAccuracy < 0.55)
				m_flAccuracy = 0.55;
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

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDirShooting = gpGlobals->v_forward;

		if (!m_iClip)
		{
			ExpToLineTargets(vecSrc, vecDirShooting);
		}

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1); //客户端播放动画
#endif
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);//获取前，右，上三个正交方向向量的函数，计算子弹射线的终点，起始点是眼睛，也就是枪口位置。

		m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME; //开枪音量传播距离
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;   //开枪照亮周围环境

		vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 2, BULLET_PLAYER_50AE, GetDamage(), 0.81, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);//武器射出去子弹，造成伤害

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		//SendWeaponAnim(3, UseDecrement() != FALSE);
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireDEAGLEAMETHYST, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;
		m_pPlayer->pev->punchangle.x -= 2;

	}

	void CDEAGLEAMETHYST::ExpToLineTargets(Vector vecSrc, Vector vecDir)
	{
#ifndef CLIENT_DLL
		TraceResult tr;
		g_engfuncs.pfnSetGroupMask(0, GROUP_OP_AND);
		m_pPlayer->pev->groupinfo = (1 << 1);

		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);
			if (!entity)
				continue;

			if (entity->edict() == m_pPlayer->edict())
				continue;

			CBasePlayer* TargetPlayer = static_cast<CBasePlayer*>(entity);

			if (g_pGameRules->PlayerRelationship(m_pPlayer, TargetPlayer) == GR_TEAMMATE)
				TargetPlayer->pev->groupinfo = (1 << 2);
		}

		UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192.0, dont_ignore_monsters, m_pPlayer->edict(), &tr);

		m_pPlayer->pev->groupinfo = 0;

		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);
			if (!entity)
				continue;

			CBasePlayer* TargetPlayer = static_cast<CBasePlayer*>(entity);
			TargetPlayer->pev->groupinfo = 0;
		}

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(tr.vecEndPos[0]);
		WRITE_COORD(tr.vecEndPos[1]);
		WRITE_COORD(tr.vecEndPos[2]);
		WRITE_SHORT(MODEL_INDEX("sprites/ame5stack.spr"));
		WRITE_BYTE(15);
		WRITE_BYTE(0.2f);
		WRITE_BYTE(TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		EMIT_SOUND_DYN(this->edict(), CHAN_STATIC, "weapons/broad-2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		if (tr.flFraction < 1.0)
		{
			TraceResult CheckHeadShotTR;
			CBaseEntity* pEntity = NULL;

			Vector vecEnd;

			while ((pEntity = UTIL_FindEntityInSphere(pEntity, tr.vecEndPos, 100.0)) != NULL)
			{
				if (pEntity->edict() == m_pPlayer->edict())
					continue;

				if (pEntity->IsDormant())
					continue;

				if (pEntity->pev->takedamage == DAMAGE_NO)
					continue;

				if (pEntity->Classify() == CLASS_NONE)
					continue;

				if (pEntity->IsPlayer())
				{
					CBasePlayer* target = static_cast<CBasePlayer*>(pEntity);

					if (g_pGameRules->PlayerRelationship(m_pPlayer, target) == GR_TEAMMATE)
						continue;
				}

				vecEnd = pEntity->BodyTarget(pev->origin);

				UTIL_TraceLine(tr.vecEndPos, vecEnd, dont_ignore_monsters, pev->owner, &CheckHeadShotTR);

				if (CheckHeadShotTR.flFraction < 1.0 && CheckHeadShotTR.pHit != pEntity->edict())
					continue;

				ClearMultiDamage();
				CheckHeadShotTR.iHitgroup = HITGROUP_CHEST;
				pEntity->TraceAttack(m_pPlayer->pev, GetExpDamage(), (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);

				ApplyMultiDamage(pev, m_pPlayer->pev);
			}

		}
#endif
	}

	float CDEAGLEAMETHYST::GetExpDamage() const
	{
		float flDamage = 20.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 75.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 36.0f;
#endif
		return flDamage;
	}
	void CDEAGLEAMETHYST::Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(DEAGLE_MAX_CLIP, 1, DEAGLE_RELOAD_TIME))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.9;
		}
	}

	void CDEAGLEAMETHYST::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
			SendWeaponAnim(0, UseDecrement() != FALSE);
	}

	float CDEAGLEAMETHYST::GetDamage() const
	{
		float flDamage = 56.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 70.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 70.0f;
#endif
		return flDamage;
	}
}
