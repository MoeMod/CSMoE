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
#include "wpn_chainsr.h"
#include "weapons/RadiusDamage.hpp"

#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifndef CLIENT_DLL
	LINK_ENTITY_TO_CLASS(chainsr_shadow, CChainSrShadow);
	CChainSrShadow* CChainSrShadow::Create(const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iType, int iLastHitGroup, float flDamage)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("chainsr_shadow"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CChainSrShadow* ChainSrShadow = (CChainSrShadow*)Instance(pent);

		if (ChainSrShadow)
		{
			ChainSrShadow->m_iType = iType;

			ChainSrShadow->pev->owner = pentOwner;
			ChainSrShadow->pev->origin = vecOrigin;
			ChainSrShadow->pev->angles = vecAngles;

			ChainSrShadow->m_pLastHitGroup = iLastHitGroup;
			ChainSrShadow->m_pLastDamage = flDamage;

			ChainSrShadow->Spawn();
		}

		return ChainSrShadow;
	}
	void CChainSrShadow::Init(CBasePlayer* pOwner, CChainSr* pWeapon, CBaseEntity* pEnemy)
	{
		m_pOwner = pOwner;
		m_pWeapon = pWeapon;
		m_iTeam = m_pOwner->m_iTeam;

		m_pEnemy = pEnemy;

		//Set Aiming AT?
		if (m_pEnemy)
		{
			Vector vecOrigin = pev->origin; Vector vecAngles;

			vecOrigin = m_pEnemy->pev->origin - vecOrigin;
			vecAngles = UTIL_VecToAngles(vecOrigin);
			vecAngles.x = vecAngles.z = 0.0;

			pev->angles = vecAngles;
			pev->v_angle = vecAngles;

		}
		pev->animtime = gpGlobals->time;

		if (m_pEnemy)
		{
			pev->sequence = 1;
			pev->frame = 0;
		}
		else
		{
			pev->sequence = 0;
			pev->frame = 0;
		}

		SetThink(&CChainSrShadow::FadeIn);
		pev->nextthink = gpGlobals->time + 1.0s;
	}
	void CChainSrShadow::Spawn(void)
	{
		Precache();

		SET_MODEL(ENT(pev), (!m_iType) ? "models/ef_chainsr_pistol.mdl" : "models/ef_chainsr_sniper.mdl");
		UTIL_SetOrigin(pev, pev->origin);

		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_TOSS;
		pev->framerate = 1.0;
		pev->frame = 0;
		pev->sequence = 0;
		pev->animtime = gpGlobals->time;

		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 0.0;

		pev->classname = MAKE_STRING("chainsr_shadow");
		m_iState = 0;
	}
	void CChainSrShadow::Precache(void)
	{
		PRECACHE_MODEL("models/ef_chainsr_pistol.mdl");
		PRECACHE_MODEL("models/ef_chainsr_sniper.mdl");

		m_iSkill[0] = PRECACHE_MODEL("sprites/ef_chainsr_skill.spr");
		m_iSkill[1] = PRECACHE_MODEL("sprites/ef_chainsr_skill2.spr");

		m_iShadowMuzzleFlash[0] = PRECACHE_MODEL("sprites/ef_chainsr_shadowshoot1.spr");
		m_iShadowMuzzleFlash[1] = PRECACHE_MODEL("sprites/ef_chainsr_shadowshoot2.spr");

		PRECACHE_SOUND("weapons/chainsr_smoke.wav");
		PRECACHE_MODEL("sprites/ef_sbmine_debuff.spr");


	}
	void EXPORT CChainSrShadow::OnThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.05s;

		if (!m_pOwner)
		{
			SUB_Remove();
			return;
		}

		if (m_pOwner->m_bIsZombie || !m_pOwner->IsAlive())
		{
			SUB_Remove();
			return;
		}

		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pOwner->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];

		if (!pWeapon)
		{
			SUB_Remove();
			return;
		}

		if (pWeapon->m_iId != WEAPON_CHAINSR)
		{
			SUB_Remove();
			return;
		}

		if (!m_iState)
		{
			if (m_pEnemy)
			{
				pev->sequence = 1;
				pev->frame = 0;

				m_iState = 1;
				pev->animtime = gpGlobals->time;
				m_flAnimTime = gpGlobals->time + 0.1s;//1.26s
				pev->renderamt = 255.0;
			}
			else
			{
				pev->sequence = 0;
				pev->frame = 0;

				pev->animtime = gpGlobals->time;
				m_flAnimTime = invalid_time_point;
				pev->renderamt = 255.0;

				SetThink(&CChainSrShadow::StartFadeOut);
				pev->nextthink = gpGlobals->time + 1.26s;
			}
		}
		else if (m_iState == 1)
		{
			if (m_flAnimTime != invalid_time_point && m_flAnimTime < gpGlobals->time)
			{
				m_flAnimTime = invalid_time_point;
				pev->animtime = gpGlobals->time;

				if (m_pEnemy)
				{
					DoSingleDamage(m_pEnemy);

					MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, this->pev);
					WRITE_BYTE(4);
					WRITE_SHORT(this->entindex());
					WRITE_BYTE(m_iType ? 23 : 24);
					MESSAGE_END();

					//UTIL_TempModel(g_vecZero, g_vecZero, g_vecZero, m_iShadowMuzzleFlash[!m_iType], 30, 0, 255, false, 0, 255, kRenderTransAdd, this, false, 0, 0, 6, 0);
					//22

				}

				SetThink(&CChainSrShadow::StartFadeOut);
				pev->nextthink = gpGlobals->time + 2.0s;
			}
		}
	}

	void CChainSrShadow::StartFadeOut()
	{
		if (pev->rendermode != kRenderTransTexture)
		{
			pev->renderamt = 255.0f;
			pev->rendermode = kRenderTransTexture;
		}
		pev->nextthink = gpGlobals->time + 0.01s;

		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
		WRITE_SHORT(entindex());
		MESSAGE_END();

		SetThink(&CChainSrShadow::FadeOut);
	}

	void CChainSrShadow::FadeOut()
	{
		if (pev->renderamt > 10)
		{
			pev->renderamt -= 10.0f;
			pev->nextthink = gpGlobals->time + 0.01s;
		}
		else
		{
			pev->renderamt = 0.0f;
			pev->nextthink = gpGlobals->time + 0.01s;
			SetThink(&CChainSrShadow::SUB_Remove);
		}
	}
	void CChainSrShadow::FadeIn()
	{
		if (pev->renderamt < 250)
		{
			pev->renderamt += 10.0f;
			pev->nextthink = gpGlobals->time + 0.01s;
		}
		else
		{
			pev->renderamt = 255.0f;
			pev->nextthink = gpGlobals->time + 0.01s;
			SetThink(&CChainSrShadow::OnThink);

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iSkill[0]);
			WRITE_BYTE(15);
			WRITE_BYTE(255);
			WRITE_BYTE(30);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z);
			WRITE_SHORT(m_iSkill[1]);
			WRITE_BYTE(15);
			WRITE_BYTE(255);
			WRITE_BYTE(10);
			MESSAGE_END();

			EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/chainsr_smoke.wav", VOL_NORM, ATTN_NORM);
		}
	}
	void CChainSrShadow::DoSingleDamage(CBaseEntity* pEnemy)
	{

		Vector vecDirection = (pEnemy->pev->origin - pev->origin).Normalize();
		Vector vecAttachment, vecAttachmentAngles;
		GET_ATTACHMENT(ENT(pev), 0, vecAttachment, vecAttachmentAngles);


		TraceResult tr;
		UTIL_TraceLine(vecAttachment, pEnemy->pev->origin + Vector{ 0,0,16 }, missile, ENT(m_pOwner->pev), &tr);

		//UTIL_BeamPointsStretch(vecAttachment, pEnemy->pev->origin + Vector{ 0,0,16 }, MODEL_INDEX("sprites/laserbeam.spr"),
		//	30, 10, 255, 178, 0, 5, 255, 0);

#ifndef CLIENT_DLL
		ClearMultiDamage();
		tr.iHitgroup = m_pLastHitGroup;
		pEnemy->TraceAttack(m_pOwner->pev, GetDamage(m_iType) * (m_iType? 1.0f : 0.9f), vecDirection, &tr, DMG_NEVERGIB | DMG_BULLET);
		ApplyMultiDamage(this->pev, m_pOwner->pev);
#endif
		if (pEnemy)
		{
			if (pEnemy && (pEnemy->IsPlayer() || pEnemy->Classify() == CLASS_PLAYER_ALLY))
			{
				if (pEnemy->IsPlayer())
				{

					CBasePlayer* player = dynamic_cast<CBasePlayer*>(pEnemy);

					if (g_pGameRules->PlayerRelationship(m_pOwner, pEnemy) != GR_TEAMMATE)
					{
						MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, player->pev->origin);
						WRITE_BYTE(TE_KILLENTITYATTACHMENTS);
						WRITE_SHORT(player->entindex());
						MESSAGE_END();

						MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, player->pev);
						WRITE_BYTE(4);
						WRITE_SHORT(player->entindex());
						WRITE_BYTE(22);
						MESSAGE_END();

						player->pev->velocity = g_vecZero;
						player->m_tLockBuffTime = gpGlobals->time + 2.5s;
						player->m_flLockGravity = 100.0f;
						player->m_flLockSpeed = 80.0f;
						player->ResetMaxSpeed();
					}
				}
			}
		}

		EMIT_SOUND(ENT(pev), CHAN_AUTO, "weapons/chainsr-1.wav", 0.5, ATTN_NORM);

	}

	float CChainSrShadow::GetDamage(int iType) const
	{
		float flDamage = 120.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flDamage = iType ? 4400 : 3960;
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 15600;
#endif
		return flDamage;
	}

#endif
	LINK_ENTITY_TO_CLASS(weapon_chainsr, CChainSr)

		void CChainSr::Spawn(void)
	{
		pev->classname = MAKE_STRING("weapon_chainsr");

		Precache();
		m_iId = WEAPON_CHAINSR;
		SET_MODEL(ENT(pev), "models/w_chainsr.mdl");

		m_iDefaultAmmo = CHAINSR_MAX_CLIP;
		FallInit();
	}

	void CChainSr::Precache(void)
	{
		PRECACHE_MODEL("models/v_chainsr.mdl");
		PRECACHE_MODEL("models/p_chainsr.mdl");
		PRECACHE_MODEL("models/w_chainsr.mdl");

		//PRECACHE_MODEL("models/ef_chainsr_wave.mdl");

		PRECACHE_MODEL("sprites/muzzleflash270.spr");
		PRECACHE_MODEL("sprites/muzzleflash271.spr");
		PRECACHE_MODEL("sprites/muzzleflash272.spr");

		PRECACHE_MODEL("sprites/ef_sbmine_debuff.spr");
		PRECACHE_MODEL("models/ef_scorpion_hole.mdl");


		m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell_big.mdl");
		m_usFireChainSr = PRECACHE_EVENT(1, "events/chainsr.sc");
	}

	int CChainSr::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "50BMG";
		p->iMaxAmmo1 = MAX_AMMO_50BMG;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = CHAINSR_MAX_CLIP;
		p->iSlot = 0;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_CHAINSR;
		p->iFlags = 0;
		p->iWeight = AWP_WEIGHT;

		return 1;
	}

	BOOL CChainSr::Deploy(void)
	{
#ifndef CLIENT_DLL

		m_pPlayer->UpdateShieldCrosshair(false);
#endif
		if (DefaultDeploy("models/v_chainsr.mdl", "models/p_chainsr.mdl", CHAINSR_DRAW, "rifle", UseDecrement() != FALSE, 0.73s, 1.0s))
		{
			m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
			return TRUE;
		}

		return FALSE;
	}

	void CChainSr::SecondaryAttack(void)
	{
		switch (m_pPlayer->m_iFOV)
		{
		case 90:
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 35;
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(true);
#endif
			break;
		default:
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(false);
#endif
			break;
		}

		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
	}

	void CChainSr::PrimaryAttack(void)
	{

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			ChainSrFire(0.01, 1.26s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 120)
			ChainSrFire(0.01, 1.26s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 10)
			ChainSrFire(0.005, 1.26s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			ChainSrFire(0.0, 1.26s, FALSE);
		else
			ChainSrFire(0.0005, 1.26s, FALSE);

	}

	void CChainSr::ChainSrFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->m_bResumeZoom = true;
#ifndef CLIENT_DLL
			m_pPlayer->UpdateShieldCrosshair(m_pPlayer->m_bResumeZoom);
#endif
			m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
		}
		else // not zoomed, so bullets diverts more
			flSpread += 0.01;

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

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.55s;
		m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 9, BULLET_PLAYER_338MAG, GetDamage(), CHAINSR_RANGE_MODIFER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

		PLAYBACK_EVENT_FULL(0, ENT(m_pPlayer->pev), m_usFireChainSr, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
		m_pPlayer->pev->punchangle.x -= 12;
	}

#ifndef CLIENT_DLL
	void CChainSr::FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
	{
		if (CBaseEntity* pEntity = gMultiDamage.pEntity)
		{
			if (pEntity && (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY))
			{
				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = dynamic_cast<CBasePlayer*>(pEntity);

					if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
					{

						MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, player->pev);
						WRITE_BYTE(4);
						WRITE_SHORT(player->entindex());
						WRITE_BYTE(22);
						MESSAGE_END();

						player->pev->velocity = g_vecZero;
						player->m_tLockBuffTime = gpGlobals->time + 2.5s;
						player->m_flLockGravity = 100.0f;
						player->m_flLockSpeed = 80.0f;
						player->ResetMaxSpeed();
					}
				}
			}
		}
	}
#endif
	void CChainSr::EntityTakeDamageCallBack(int iWpnID, CBaseEntity* pVictim, entvars_t* pevInflictor, CBasePlayer* pAttacker, float flDamage, int bitsDamageType)
	{
		//pVictim->被击杀的人  pevInflictor->击杀者使用的武器/实体  pAttacker->击杀者
		//谨慎在callback中使用return... 

		if (pAttacker && pAttacker->IsAlive())
		{
			if (iWpnID == WEAPON_CHAINSR)
			{
				CBasePlayer* pVictimPlayer = nullptr;
				if (pVictim && pVictim->IsPlayer())
					pVictimPlayer = static_cast<CBasePlayer*>(pVictim);

				auto CheckActiveItem = [](CBasePlayer* pPlayer)
				{
					return (pPlayer->m_pActiveItem != NULL && pPlayer->m_pActiveItem->m_iId == WEAPON_CHAINSR) ? true : false;
				};
				//Ensure the victim can be kill?
				if (pVictim->pev->takedamage != DAMAGE_NO /* && (pVictimPlayer && !(pVictimPlayer->m_iBuff & BUFF_REVIVE))*/)
				{
					//If the damage amount >= victim's health ->deathmsg...
					float flVictimHealth = pVictim->pev->health;
					CBaseEntity* pEnt = CBaseEntity::Instance(pevInflictor);


					if ((pEnt && !Q_strcmp(STRING(pEnt->pev->classname), "chainsr_shadow")) || (ENT(pevInflictor) == this->pev->owner) && CheckActiveItem(pAttacker))
					{
						if (flDamage >= flVictimHealth)
						{
							if ((pEnt && pEnt->pev->owner == ENT(pAttacker->pev)) || this->pev->owner == ENT(pAttacker->pev))
							{
								std::string SzPevInflictor;
								if (pVictimPlayer != nullptr)
								{
									SzPevInflictor = (ENT(pevInflictor) == this->pev->owner) ? "Use Weapon to Defeat Enemy[Player]" : "Use Shadow Assistant to Defeat Enemy[Player]";
									//ClientPrint(pAttacker->pev, HUD_PRINTTALK, "%s -> %s ", SzPevInflictor.c_str(), STRING(pVictim->pev->netname));
								}
								else
								{
									SzPevInflictor = (ENT(pevInflictor) == this->pev->owner) ? "Use Weapon to Defeat Enemy[NPC]" : "Use Shadow Assistant to Defeat Enemy[NPC]";
									//ClientPrint(pAttacker->pev, HUD_PRINTTALK, "%s -> %s %s ", SzPevInflictor.c_str(), std::to_string(pVictim->entindex()).c_str(), STRING(pVictim->pev->classname));
								}
								ShadowSearchForEnemy(pVictim, pVictim->pev->origin, 1, flDamage, pVictim->m_LastHitGroup);
							}
						}
						//If the damage amount < victim's health, push the victim into vector and consider this damage as assistance...
						else
						{
							//If the vector does not contain the specific, push it!
							if (!std::count(pVictim->m_vecTempAttackers.begin(), pVictim->m_vecTempAttackers.end(), m_pPlayer))
								pVictim->m_vecTempAttackers.push_back(m_pPlayer);
						}
					}
					else
					{
						if (this->pev->owner != ENT(pAttacker->pev))
						{
							if (flDamage >= flVictimHealth)
							{

							}
						}

					}
				}
			}
		}
	}

	void CChainSr::WpnDeathCallBack(CBaseEntity* pVictim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
	{

#ifndef CLIENT_DLL

		CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);

		if (m_pPlayer->pev != pevAttacker)
		{
			CBaseEntity* pEnt = CBaseEntity::Instance(pevInflictor);

			//ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "Assist Defeat With:%s -> %s ", STRING(pAttacker->pev->netname), STRING(pVictim->pev->netname));

			ShadowSearchForEnemy(pVictim, pVictim->pev->origin, 0, flDamage, pVictim->m_LastHitGroup);
		}
#endif

	}

	void CChainSr::ShadowSearchForEnemy(CBaseEntity* pIgnored, Vector vecOrigin, int iType, float flDamage, int iLastHitGroup)
	{
#ifndef CLIENT_DLL

		float flMinDistance = 8192 * 8192;

		CBaseEntity* iIdDetected = NULL;
		CBaseEntity* pEntity = NULL;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecOrigin, ChainSr_ShadowSearchRadius)) != NULL)
		{
			if (pEntity->IsDormant())
				continue;

			if (pEntity->Classify() != CLASS_PLAYER_ALLY && !pEntity->IsPlayer())
			{
				if (!FClassnameIs(pEntity->pev, "monster_entity"))
					continue;
			}

			if (pEntity->pev->takedamage == DAMAGE_NO)
				continue;

			if (pEntity == m_pPlayer)
				continue;

			if (pEntity == pIgnored)
				continue;

			//队友不触发检测
			if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
				continue;

			if (UTIL_IsWallBetweenEntity(pEntity, pIgnored))
				continue;

			//iIdDetected = pEntity;

			float flDistance = (pEntity->pev->origin - vecOrigin).Length();
			if (flMinDistance > flDistance)
			{
				flMinDistance = flDistance;
				iIdDetected = pEntity;
			}
		}

		Vector vecDir = (vecOrigin - (iIdDetected ? (iIdDetected->pev->origin) : vecOrigin));
		Vector vecTemp;
		Vector vecScale;
		if (vecDir.Length())
		{
			vecDir = vecDir.Normalize();
			vecTemp = vecDir;

			VectorScale(vecTemp, -32, vecScale);
			VectorAdd(vecOrigin, vecScale, vecOrigin);
		}

		CChainSrShadow* ChainSrShadow = CChainSrShadow::Create(vecOrigin, pev->angles, ENT(m_pPlayer->pev), iType, iLastHitGroup, flDamage);

		if (ChainSrShadow)
			ChainSrShadow->Init(m_pPlayer, this, iIdDetected);

		//if (iIdDetected)
		//	ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "Nearest Enemy Searched : %s [Distance:%s m]", STRING(iIdDetected->pev->netname), std::to_string((flMinDistance / 39.37)).c_str());
#endif
	}
	void CChainSr::ItemPostFrame(void)
	{

		return CBasePlayerWeapon::ItemPostFrame();
	}

	void CChainSr::Reload(void)
	{
		if (DefaultReload(CHAINSR_MAX_CLIP, CHAINSR_RELOAD, CHAINSR_RELOAD_TIME))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			if (m_pPlayer->pev->fov != 90)
			{
				m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 10;
				SecondaryAttack();
			}
			SetThink(&CChainSr::ReloadingWaveThink);
			pev->nextthink = gpGlobals->time + 0.6s;
		}
	}

	void CChainSr::ReloadingWaveThink(void)
	{
		PLAYBACK_EVENT_FULL(0, ENT(m_pPlayer->pev), m_usFireChainSr, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, TRUE, FALSE);

#ifndef CLIENT_DLL
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

		struct
		{
			float RadiusDamageRadius = 6.75 * 39.37;
			int RadiusDamageBitsDamageType = DMG_BULLET;
			bool RadiusDamageCanHeadshot = true;
			float RadiusDamageAmount = 0.0f;
		} Data;

		std::vector<CBaseEntity*> out;
		RadiusDamage(Data, vecSrc, m_pPlayer, m_pPlayer, std::back_inserter(out));
		for (CBaseEntity* ent : out)
		{
			CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(ent);
			if (player && player->m_bIsZombie)
			{
				Vector vecDir = (player->pev->origin - m_pPlayer->pev->origin);
				vecDir.z = 0;

				ApplyKnockbackData(player, player->Center() - vecSrc, { 3000, 3000, 3000, 3000, 1.0f });
				player->pev->velocity.z = +200.0f;
			}
		}
#endif

		pev->nextthink = gpGlobals->time + 0.1s;
		SetThink(nullptr);
	}
	void CChainSr::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		if (m_iClip)
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
			SendWeaponAnim(CHAINSR_IDLE, UseDecrement() != FALSE);
		}
	}

	float CChainSr::GetMaxSpeed(void)
	{
		return CHAINSR_MAX_SPEED;
	}

	float CChainSr::GetDamage() const
	{
		float flDamage = 150.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			if (enhancedzombie.value)
				flDamage = 3100;
			else
				flDamage = 1100;
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 6200.0f;
#endif
		return flDamage;
	}
}
