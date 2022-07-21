#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_divinetitan.h"

#include "monsters.h"
#include "gamerules.h"

#include <vector>
#include <array>

#ifndef CLIENT_DLL
#include "player/player_mod_strategy.h"
#include "gamemode/mods.h"
#endif
#include "weapons/KnifeAttack.h"


#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(weapon_divinetitan, CDivineTitan)

	void CDivineTitan::Spawn(void)
	{
		pev->classname = MAKE_STRING("weapon_divinetitan");

		Precache();
		m_iId = WEAPON_DIVINETITAN;
		SET_MODEL(ENT(pev), "models/w_divinetitan.mdl");
		m_iClip = -1;
		m_iDefaultAmmo = 20;

		m_iShotsFired = 0;
		m_bDelayFire = false;
		InitChargeState();

		FallInit();
	}

	void CDivineTitan::Precache(void)
	{
		PRECACHE_MODEL("models/v_divinetitan.mdl");
		PRECACHE_MODEL("models/p_divinetitan.mdl");
		PRECACHE_MODEL("models/w_divinetitan.mdl");

		PRECACHE_SOUND("weapons/divinetitan_A_mode-1_start.wav");
		PRECACHE_SOUND("weapons/divinetitan_A_mode-1_end.wav");
		PRECACHE_SOUND("weapons/divinetitan_A_mode-1_loop.wav");

		PRECACHE_SOUND("weapons/divinetitan_B_mode-2.wav");
		PRECACHE_SOUND("weapons/divinetitan_B_mode-2_exp.wav");

		PRECACHE_SOUND("weapons/divinetitan_charge.wav");
		PRECACHE_SOUND("weapons/divinetitan_idle.wav");
		PRECACHE_SOUND("weapons/divinetitan_idle_B_mode.wav");
		PRECACHE_SOUND("weapons/divinetitan_idle_pullpin.wav");

		PRECACHE_MODEL("sprites/muzzleflash248.spr");
		PRECACHE_MODEL("sprites/muzzleflash250.spr");
		PRECACHE_MODEL("sprites/muzzleflash251.spr");
		PRECACHE_MODEL("sprites/muzzleflash252.spr");
	}

	int CDivineTitan::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "titanenergy";
		p->iMaxAmmo1 = 100;
		p->pszAmmo2 = "titanenergy2";
		p->iMaxAmmo2 = 1;
		p->iMaxClip = WEAPON_NOCLIP;
		p->iSlot = 3;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_DIVINETITAN;
		p->iWeight = HEGRENADE_WEIGHT;
		p->iFlags = 0;


		return 1;
	}

	int CDivineTitan::ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		if (m_iDefaultAmmo)
		{
			if (m_pPlayer)
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
			}
			m_iClip = WEAPON_NOCLIP;
			m_iDefaultAmmo = 0;
			return TRUE;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	BOOL CDivineTitan::Deploy(void)
	{
		ResetChargeTimer();
		m_fMaxSpeed = 250;
		m_pPlayer->m_bShieldDrawn = false;
		int m_iDeployAnim = WEAPON_ANIM_DRAW1;
		switch (m_iWpnState)
		{
		case 0:
		{
			m_iDeployAnim = WEAPON_ANIM_DRAW1;
			break;
		}
		case 1:
		{
			m_iDeployAnim = WEAPON_ANIM_DRAW2;
			break;
		}
		case 3:
		{
			m_iDeployAnim = WEAPON_ANIM_DRAW3;
			break;
		}
		}
		m_bCanThrow = true;
		m_pPlayer->m_flPlayerBuffData = m_iWpnState;


		return DefaultDeploy("models/v_divinetitan.mdl", "models/p_divinetitan.mdl", m_iDeployAnim, "grenade", UseDecrement() != FALSE);
	}

	void CDivineTitan::Holster(int skiplocal)
	{
		ResetChargeTimer();
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void CDivineTitan::PrimaryAttack(void)
	{

		int m_iPrimaryAnim = WEAPON_ANIM_DEFAULTTOA;
		duration_t m_iPrimaryAttackTime = WEAPON_TIMEA_ON;
		duration_t m_iPrimaryAnimTime = WEAPON_ANIM_DEFAULTTOA_TIME;

		switch (m_iWpnState)
		{
		case 0:
		{
			m_iPrimaryAttackTime = WEAPON_TIMEA_ON;
			m_iPrimaryAnimTime = WEAPON_ANIM_DEFAULTTOA_TIME;
			m_iPrimaryAnim = WEAPON_ANIM_DEFAULTTOA;

#ifndef CLIENT_DLL
			CDivineTitanBase* pTitanBase = CDivineTitanBase::Create(1, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState);
#endif

			m_iWpnState = 1;
			break;
		}
		case 1:
		{
			m_iPrimaryAttackTime = WEAPON_TIMEA_OFF;
			m_iPrimaryAnimTime = WEAPON_ANIM_ATODEFAULT_TIME;
			m_iPrimaryAnim = WEAPON_ANIM_ATODEFAULT;

			m_iWpnState = 0;
			break;
		}
		default:
		{
			m_iPrimaryAttackTime = WEAPON_TIMEABSWITCH;
			m_iPrimaryAnimTime = WEAPON_ANIM_BTOA_TIME;
			m_iPrimaryAnim = WEAPON_ANIM_BTOA;


			m_iWpnState = 1;
			break;
		}
		}
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + m_iPrimaryAnimTime;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + m_iPrimaryAttackTime;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + m_iPrimaryAttackTime;
		SendWeaponAnim(m_iPrimaryAnim, UseDecrement() != FALSE);
		m_pPlayer->m_flPlayerBuffData = m_iWpnState;

		ResetChargeTimer();
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iWeaponState;

	}


	void CDivineTitan::SecondaryAttack(void)
	{
		int m_iSecondaryAnim = WEAPON_ANIM_DEFAULTTOB;
		duration_t m_iSecondaryAttackTime = WEAPON_TIMEB_ON;
		duration_t m_iSecondaryAnimTime = WEAPON_ANIM_DEFAULTTOB_TIME;

		switch (m_iWpnState)
		{
		case 0:
		{
			m_iSecondaryAttackTime = WEAPON_TIMEB_ON;
			m_iSecondaryAnimTime = WEAPON_ANIM_DEFAULTTOB_TIME;
			m_iSecondaryAnim = WEAPON_ANIM_DEFAULTTOB;

#ifndef CLIENT_DLL
			CDivineTitanBase* pTitanBase = CDivineTitanBase::Create(1, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState);
#endif

			m_iWpnState = 2;
			break;
		}
		case 1:
		{
			m_iSecondaryAttackTime = WEAPON_TIMEABSWITCH;
			m_iSecondaryAnimTime = WEAPON_ANIM_ATOB_TIME;
			m_iSecondaryAnim = WEAPON_ANIM_ATOB;

			m_iWpnState = 2;
			break;
		}
		default:
		{
			m_iSecondaryAttackTime = WEAPON_TIMEB_OFF;
			m_iSecondaryAnimTime = WEAPON_ANIM_BTODEFAULT_TIME;
			m_iSecondaryAnim = WEAPON_ANIM_BTODEFAULT;

			m_iWpnState = 0;
			break;
		}
		}
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + m_iSecondaryAnimTime;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + m_iSecondaryAttackTime;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + m_iSecondaryAttackTime;
		SendWeaponAnim(m_iSecondaryAnim, UseDecrement() != FALSE);
		m_pPlayer->m_flPlayerBuffData = m_iWpnState;

		ResetChargeTimer();
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iWeaponState;
	}
	void CDivineTitan::ItemPostFrame(void)
	{
		if (m_iWeaponState == 100)
			m_flTimeNextCharge = invalid_time_point;

		if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState += int((gpGlobals->time - m_flTimeNextCharge) / 0.25s + 1.0);

			if (m_iWeaponState >= 100)
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState = 100;

			m_flTimeNextCharge = gpGlobals->time + 0.25s;
		}

		if (m_flPlayerThink <= gpGlobals->time && m_iWpnState)
		{
			if (m_iWpnState && !m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
				m_iWpnState = m_pPlayer->m_flPlayerBuffData = 0;
				m_flPlayerThink = invalid_time_point;
			}
			if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
			else
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;

			m_flPlayerThink = gpGlobals->time + 0.25s;
			m_flTimeNextCharge = invalid_time_point;
		}

		m_iButton = m_pPlayer->pev->button;

		if (m_iButton & IN_RELOAD && m_bCanThrow)
		{
			SendWeaponAnim(WEAPON_ANIM_PULLPIN, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.36s;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.36s;

			m_iAutoAttackStage = 1;
			m_bCanThrow = false;
		}
		if (m_flNextPrimaryAttack >= UTIL_WeaponTimeBase())
			return CBasePlayerWeapon::ItemPostFrame();

		switch (m_iAutoAttackStage)
		{
		case 1:
		{
			SendWeaponAnim(WEAPON_ANIM_IDLE_PULLPIN, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.33s;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.33s;

			m_iAutoAttackStage = 2;
			break;

		}
		case 2:
		{
			Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL
			CDivineTitanSummon* pTitanBall = CDivineTitanSummon::Create(0, false, NULL, vecSrc, g_vecZero, pev->owner, m_iTeam);
			if (pTitanBall)
			{
				pTitanBall->pev->velocity = gpGlobals->v_forward * 1000;
			}
#endif
			SendWeaponAnim(WEAPON_ANIM_THROW, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5s;

			m_iAutoAttackStage = 0;
			m_bCanThrow = true;
			break;
		}
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}


	void CDivineTitan::WeaponIdle(void)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
		int m_iIdleAnim = WEAPON_ANIM_IDLE1;
		duration_t m_flIdleTime = WEAPON_ANIM_IDLE1_TIME;

		switch (m_iWpnState)
		{
		case 0:
		{
			m_iIdleAnim = WEAPON_ANIM_IDLE1;
			m_flIdleTime = WEAPON_ANIM_IDLE1_TIME;
			break;
		}
		case 1:
		{
			m_iIdleAnim = WEAPON_ANIM_IDLE2;
			m_flIdleTime = WEAPON_ANIM_IDLE2_TIME;
			break;
		}
		case 2:
		{
			m_iIdleAnim = WEAPON_ANIM_IDLE3;
			m_flIdleTime = WEAPON_ANIM_IDLE3_TIME;
			break;
		}
		}
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + m_flIdleTime;
		SendWeaponAnim(m_iIdleAnim, UseDecrement() != FALSE);
		ResetChargeTimer();

	}
	void CDivineTitan::ResetChargeTimer(void)
	{
		if (m_iWeaponState != 100 && m_flTimeNextCharge == invalid_time_point)
			m_flTimeNextCharge = gpGlobals->time + 1.0s;
	}
	void CDivineTitan::InitChargeState(void)
	{
		m_iWeaponState = 0;
		m_flTimeNextCharge = invalid_time_point;
	}





#ifndef CLIENT_DLL
	LINK_ENTITY_TO_CLASS(divinetitan_summoner, CDivineTitanSummon)

		CDivineTitanSummon* CDivineTitanSummon::Create(int iType, bool iIsEffect, int iEffectType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("divinetitan_summoner"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CDivineTitanSummon* pSummon = (CDivineTitanSummon*)Instance(pent);

		if (pSummon)
		{
			if (iIsEffect)
				pSummon->m_iEffectType = iEffectType;

			pSummon->m_iType = iType;
			pSummon->pev->owner = pentOwner;
			pSummon->pev->origin = vecOrigin;
			pSummon->pev->angles = vecAngles;
			pSummon->m_iTeam = iTeam;
			pSummon->Spawn();
		}

		return pSummon;
	}

	void CDivineTitanSummon::Precache(void)
	{
		PRECACHE_MODEL("models/ef_divinetitan_summon.mdl");

		m_iEffect[0] = PRECACHE_MODEL("sprites/ef_divinetitan_summon0.spr");
		m_iEffect[1] = PRECACHE_MODEL("sprites/ef_divinetitan_summon1.spr");
		m_iEffect[2] = PRECACHE_MODEL("sprites/ef_divinetitan_summon2.spr");

		PRECACHE_SOUND("weapons/divinetitan_exp.wav");
		PRECACHE_SOUND("weapons/divinetitan_fx.wav");

	}

	void CDivineTitanSummon::Spawn()
	{
		Precache();

		//Init For the thrown ball before triggering.
		if (!m_iType)
		{
			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_BBOX;
			pev->body = 0;
			pev->frame = 0.0;

			SET_MODEL(ENT(pev), "models/w_divinetitan.mdl");
			UTIL_SetSize(pev, Vector(-3, -4, 47), Vector(3, 4, 55));
			//UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

			SetThink(&CDivineTitanSummon::OnThink);
			SetTouch(&CDivineTitanSummon::OnTouch);
		}
		//Init For Titan
		else if (m_iType == 1)
		{
			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_NOT;
			pev->body = 0;
			pev->frame = 0.0;
			pev->framerate = 1.0;

			//Just No rendermode before fadeout?
			pev->rendermode = kRenderTransTexture;
			pev->renderamt = 255;
			//Animation time from Fps/Model ->Time For Forced attraction(51.0f/31.0f)
			m_flNextThinkPoint[0] = gpGlobals->time + 1.65s;


			pev->animtime = gpGlobals->time;
			pev->nextthink = gpGlobals->time + 0.05s;

			EMIT_SOUND_DYN(ENT(this->pev), CHAN_AUTO, "weapons/divinetitan_fx.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
			SET_MODEL(ENT(pev), "models/ef_divinetitan_summon.mdl");
			UTIL_SetSize(pev, Vector(-1, -1, -1), Vector(1, 1, 1));

			SetThink(&CDivineTitanSummon::OnThink);
		}
		//Summoner Effects
		else if (m_iType == 2)
		{

			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_NOT;
			pev->scale *= 2.0;
			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 255;
			pev->frame = 1.0;

			pev->framerate = 1.0;
			pev->animtime = gpGlobals->time;
			pev->nextthink = gpGlobals->time + 0.01s;
			pev->effects |= EF_NOCULL;


			switch (m_iEffectType)
			{
			case 1:
			{
				m_flEffRemoveTime = gpGlobals->time + 1.83s;
				SET_MODEL(ENT(pev), "sprites/ef_divinetitan_summon0.spr");
				break;
			}
			case 2:
			{
				m_flEffRemoveTime = gpGlobals->time + 2.0s;
				SET_MODEL(ENT(pev), "sprites/ef_divinetitan_summon1.spr");
				break;
			}
			case 3:
			{
				m_flEffRemoveTime = gpGlobals->time + 2.0s;
				SET_MODEL(ENT(pev), "sprites/ef_divinetitan_summon2.spr");
				break;
			}
			}
			UTIL_SetSize(pev, Vector(-1, -1, -1), Vector(1, 1, 1));
			SetThink(&CDivineTitanSummon::OnThink);

		}
	}
	void CDivineTitanSummon::OnThink()
	{
		if (!m_iType)
		{
			if (pev->iuser1 == 1 && m_flNextDamagePoint[0] != invalid_time_point)
			{
				pev->nextthink = gpGlobals->time + 0.3s;
				if (m_flNextDamagePoint[0] >= gpGlobals->time)
				{

					//ForceStage
					CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
					CBasePlayer* pAttackPlayer = nullptr;
					if (pAttacker && pAttacker->IsPlayer())
						pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

					if (pAttackPlayer && !pAttackPlayer->m_bIsZombie)
					{
						TraceResult tr;
						CBaseEntity* pEntity = NULL;

						Vector vecDelta;
						Vector vecEnd;

						while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, GetRange_SummonerAttack())) != NULL)
						{
							if (pEntity->edict() == pev->owner)
								continue;

							if (pEntity->IsDormant())
								continue;

							if (pEntity->pev->takedamage == DAMAGE_NO)
								continue;

							if (pEntity->Classify() == CLASS_NONE)
								continue;

							if (pEntity->IsPlayer())
							{
								CBasePlayer* player = (CBasePlayer*)pEntity;

								if (!player)
									continue;

								if (pAttackPlayer->m_iTeam == player->m_iTeam)
									continue;

								vecEnd = player->BodyTarget(pev->origin);

								UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

								if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
									continue;
								vecDelta = pEntity->pev->origin - pev->origin;

								Vector vecPullVelocity, vecOrigin;
								const float power = 300;
								vecOrigin = pev->origin;

								vecOrigin[0] += RANDOM_LONG(-35, 35);
								vecOrigin[1] += RANDOM_LONG(-20, 20);
								vecOrigin[2] += RANDOM_LONG(-10, 10);


								vecPullVelocity = (vecOrigin - player->pev->origin).Normalize() * power;
								player->pev->velocity = vecPullVelocity;

								if (vecDelta.Length() > 360)
									continue;

								player->TakeDamage(pev, pAttackPlayer->pev, GetDamage_SummonerAttack(1), DMG_EXPLOSION | DMG_NEVERGIB);
							}
							else
							{
								vecEnd = pEntity->BodyTarget(pev->origin);

								UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

								if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
									continue;

								vecDelta = pEntity->pev->origin - pev->origin;

								if (vecDelta.Length() > 360)
									continue;

								pEntity->TakeDamage(pev, pAttackPlayer->pev, GetDamage_SummonerAttack(1), DMG_EXPLOSION | DMG_NEVERGIB);
							}
						}
					}
				}
				else
				{
					m_flNextDamagePoint[0] = invalid_time_point;
					//fps 51 - 33 - 10???
					m_flNextDamagePoint[1] = gpGlobals->time + 0.05s;
					pev->iuser1 = 2;

				}
			}
			else if (pev->iuser1 == 2 && m_flNextDamagePoint[1] != invalid_time_point)
			{
				pev->nextthink = gpGlobals->time + 0.01s;

				if (m_flNextDamagePoint[1] <= gpGlobals->time)
				{
					CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
					CBasePlayer* pAttackPlayer = nullptr;
					if (pAttacker && pAttacker->IsPlayer())
						pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

					if (pAttackPlayer && !pAttackPlayer->m_bIsZombie)
					{
						TraceResult tr;
						CBaseEntity* pEntity = NULL;

						Vector vecEnd;
						int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);

						while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, GetRange_SummonerAttack())) != NULL)
						{
							if (pEntity->edict() == pev->owner)
								continue;

							if (pEntity->IsDormant())
								continue;

							if (pEntity->pev->takedamage == DAMAGE_NO)
								continue;

							if (pEntity->Classify() == CLASS_NONE)
								continue;

							if ((bInWater && pEntity->pev->waterlevel == 0) || (!bInWater && pEntity->pev->waterlevel == 3))
								continue;

							if (g_pGameRules->PlayerRelationship(pAttackPlayer, pEntity) == GR_TEAMMATE)
								continue;

							if (pEntity->IsPlayer())
							{
								CBasePlayer* player = (CBasePlayer*)pEntity;

								if (!player)
									continue;

								vecEnd = player->BodyTarget(pev->origin);

								UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

								if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
									continue;

								player->TakeDamage(this->pev, pAttackPlayer->pev, GetDamage_SummonerAttack(0), DMG_EXPLOSION | DMG_NEVERGIB);

								if (player->m_bIsZombie && ~(player->m_iBuff) & BUFF_IGNORE_KNOCKBACK)
								{
									auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
									if (strategy)
									{
										strategy->SetStunSpeedTime(1.0s, 150.0f);	//150
										strategy->SetStunGravityTime(1.0s);
									}
								}
							}
							else
							{
								vecEnd = pEntity->BodyTarget(pev->origin);

								UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

								if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
									continue;

								pEntity->TakeDamage(this->pev, pAttackPlayer->pev, GetDamage_SummonerAttack(0), DMG_EXPLOSION | DMG_NEVERGIB);
							}
						}
					}
					m_flNextDamagePoint[1] = invalid_time_point;
					SetThink(NULL);
					UTIL_Remove(this);
				}

			}

		}
		//Check it
		if (m_iType == 1)
		{
			pev->nextthink = gpGlobals->time + 0.05s;
			if (m_flNextThinkPoint[0] <= gpGlobals->time)
			{
				if (m_flNextThinkPoint[1] == invalid_time_point)
				{
					Vector vecOrigin = m_vSaveOrigin;
					MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
					WRITE_BYTE(TE_TEMPSPRITE);
					WRITE_COORD(pev->origin.x);
					WRITE_COORD(pev->origin.y);
					WRITE_COORD(pev->origin.z + 135.0f);
					WRITE_SHORT(m_iEffect[2]);
					WRITE_BYTE(10);
					WRITE_BYTE(255);
					WRITE_BYTE(30);
					MESSAGE_END();

					MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
					WRITE_BYTE(TE_TEMPSPRITE);
					WRITE_COORD(pev->origin.x);
					WRITE_COORD(pev->origin.y);
					WRITE_COORD(pev->origin.z + 100.0f);
					WRITE_SHORT(m_iEffect[1]);
					WRITE_BYTE(8);
					WRITE_BYTE(255);
					WRITE_BYTE(25);
					MESSAGE_END();

					/*vecOrigin.z += 30;
					CDivineTitanSummon* pSummonEff2 = CDivineTitanSummon::Create(2, true, 2, vecOrigin, vecAngle, ENT(pev), m_iTeam);
					vecOrigin.z -= 10;
					CDivineTitanSummon* pSummonEff3 = CDivineTitanSummon::Create(2, true, 3, vecOrigin, vecAngle, ENT(pev), m_iTeam);*/
				}
				//91fps - 71fps
				m_flNextThinkPoint[1] = gpGlobals->time + 0.7s + 0.1s;
				m_flNextThinkPoint[0] = invalid_time_point;
			}
			if (m_flNextThinkPoint[1] > gpGlobals->time)
			{
				if (pev->rendermode == kRenderTransTexture)
				{
					pev->renderamt = 255.0f;
					pev->rendermode = kRenderTransAdd;
				}
				float flRenerment;
				flRenerment = pev->renderamt;
				if (flRenerment - 10.0f > 0.0f)
				{
					flRenerment -= 10.0f;
				}
				else
				{
					flRenerment = 0.0f;
					m_flNextThinkPoint[1] = invalid_time_point;

					pev->nextthink = gpGlobals->time + 0.6s;
					SetThink(&CBaseEntity::SUB_Remove);
				}
				pev->renderamt = flRenerment;
			}
		}
	}
	void CDivineTitanSummon::OnTouch(CBaseEntity* pOther)
	{
		if (!pOther)
			return;
		if (pOther->edict() == pev->owner)
			return;
		if (m_iType)
			return;

		Vector vecDir = pev->velocity.Normalize();
		Vector vecOrigin = pev->origin;
		vecOrigin -= vecDir * 40;
		TraceResult tr;
		Vector vecEnd;

		vecEnd = vecOrigin;
		vecEnd.z -= 8192;

		UTIL_TraceLine(vecOrigin, vecEnd, dont_ignore_monsters, ENT(pev), &tr);
		vecOrigin = tr.vecEndPos;

		CDivineTitanSummon* pTitanSummoner = CDivineTitanSummon::Create(1, false, NULL, vecOrigin, g_vecZero, pev->owner, m_iTeam);
		if (pTitanSummoner)
		{
			//m_vSaveOrigin = vecOrigin;
			//CDivineTitanSummon* pSummonEff1 = CDivineTitanSummon::Create(2, true, 1, pev->origin, g_vecZero, pev->owner, m_iTeam);

			MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_TEMPSPRITE);
			WRITE_COORD(pev->origin.x);
			WRITE_COORD(pev->origin.y);
			WRITE_COORD(pev->origin.z + 45.0f);
			WRITE_SHORT(m_iEffect[0]);
			WRITE_BYTE(10);
			WRITE_BYTE(255);
			WRITE_BYTE(60);
			MESSAGE_END();
		}
		EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "weapons/divinetitan_exp.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

		//Fps 51/30.0
		pev->nextthink = gpGlobals->time + 0.1s;
		//Check if touched
		pev->iuser1 = 1;
		m_flNextDamagePoint[0] = gpGlobals->time + 1.7s;
		m_flNextDamagePoint[1] = gpGlobals->time + 3.03s;

		SetTouch(NULL);
	}
	float CDivineTitanSummon::GetRange_SummonerAttack() const
	{
		return 300.0f;
	}
	float CDivineTitanSummon::GetDamage_SummonerAttack(int IsAttraction) const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return IsAttraction ? 500.0f : 50000.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return IsAttraction ? 250.0f : 15000.0f;
#endif
		return IsAttraction ? 5.0f : 75.0f;
	}
	LINK_ENTITY_TO_CLASS(divinetitan_base, CDivineTitanBase)
	CDivineTitanBase* CDivineTitanBase::CreateBeam(int iType, const Vector& vecStart, const Vector& vecOrigin, edict_t* pentOwner)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("divinetitan_base"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CDivineTitanBase* pBeam = (CDivineTitanBase*)Instance(pent);

		if (pBeam)
		{


			pBeam->m_iType = iType;
			pBeam->m_cStartEntity = vecStart;
			pBeam->m_vEndPosition = vecOrigin;
			pBeam->pev->origin = vecOrigin;
			pBeam->pev->owner = pentOwner;

			pBeam->Spawn();
		}

		return pBeam;
	}
	void CDivineTitanBase::ClearEnt(int iClear)
	{
		SetThink(nullptr);
		pev->effects |= EF_NODRAW; // 0x80u
		//Delete
		phs9_10_11.clear();
		return UTIL_Remove(this);
	}
	CDivineTitanBase* CDivineTitanBase::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam, int iData)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("divinetitan_base"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CDivineTitanBase* pEnt = (CDivineTitanBase*)Instance(pent);

		if (pEnt)
		{
			pEnt->m_iType = iType;
			pEnt->pev->origin = vecOrigin;
			pEnt->pev->angles = vecAngles;
			pEnt->pev->owner = pentOwner;
			pEnt->m_iTeam = iTeam;
			pEnt->m_iSaveWpnState = iData;

			pEnt->Spawn();
			
		}

		return pEnt;
	}

	void CDivineTitanBase::Precache(void)
	{
		PRECACHE_MODEL("sprites/ef_divinetitan_amode.spr");
		PRECACHE_MODEL("sprites/ef_divinetitan_amode_hit.spr");
		PRECACHE_MODEL("sprites/ef_divinetitan_bmode_explo.spr");

		PRECACHE_MODEL("models/w_divinetitan.mdl");

		PRECACHE_SOUND("weapons/divinetitan_B_mode-2_exp.wav");
		PRECACHE_SOUND("weapons/divinetitan_B_mode-2.wav");



	}

	void CDivineTitanBase::Spawn()
	{
		Precache();

		//Init For the beam.
		if (!m_iType)
		{
			pev->flags = (pev->flags | FL_CUSTOMENTITY);
			pev->rendercolor.x = 255;
			pev->rendercolor.y = 255;
			pev->rendercolor.z = 255;

			pev->renderamt = 255.0f;
			pev->body = 0;
			pev->frame = 1.0;
			pev->animtime = invalid_time_point;
			pev->scale = 200.0;

			SET_MODEL(ENT(pev), "sprites/ef_divinetitan_amode.spr");

			pev->skin = 0;
			pev->sequence = 0;
			pev->rendermode = 0;
			
			Vector m_vOrigp; m_vOrigp = m_cStartEntity;
			pev->angles = m_vOrigp;
			pev->sequence = ((pev->sequence) & 0x0FFF) | ((0 & 0xF) << 12);
			pev->skin = ((pev->skin) & 0x0FFF) | ((0 & 0xF) << 12);
			
			Vector m_VecMins, m_vecMaxs, m_vecOrigin, m_vecEndPos, m_vecStartPos;
			m_vecStartPos = m_vecOrigin = pev->origin;
			m_vecEndPos = pev->angles;
			
			m_VecMins.x = min(m_vecStartPos.x, m_vecEndPos.x);
			m_VecMins.y = min(m_vecStartPos.y, m_vecEndPos.y);
			m_VecMins.z = min(m_vecStartPos.z, m_vecEndPos.z);
			
			m_vecMaxs.x = max(m_vecStartPos.x, m_vecEndPos.x);
			m_vecMaxs.y = max(m_vecStartPos.y, m_vecEndPos.y);
			m_vecMaxs.z = max(m_vecStartPos.z, m_vecEndPos.z);
			
			m_VecMins.x = m_VecMins.x - m_vecOrigin.x;
			m_VecMins.y = m_VecMins.y - m_vecOrigin.y;
			m_VecMins.z = m_VecMins.z - m_vecOrigin.z;
			m_vecMaxs.x = m_vecMaxs.x - m_vecOrigin.x;
			m_vecMaxs.y = m_vecMaxs.y - m_vecOrigin.y;
			m_vecMaxs.z = m_vecMaxs.z - m_vecOrigin.z;
			
			pev->mins = m_VecMins;
			pev->maxs = m_vecMaxs;
			UTIL_SetSize(pev, m_VecMins, m_vecMaxs);
			pev->origin = m_vecOrigin;
			
			pev->sequence = ((pev->sequence) & 0x0FFF) | ((0 & 0xF) << 12);
			pev->skin = ((pev->skin) & 0x0FFF) | ((0 & 0xF) << 12);
			pev->body = 0;
			pev->framerate = 1.0;
			pev->animtime = gpGlobals->time + 0.875s;
			pev->nextthink = gpGlobals->time + 0.1s;
			
			SetThink(&CDivineTitanBase::OnThink);
		}
		else if (m_iType == 1)
		{
		pev->movetype = MOVETYPE_FOLLOW;
		pev->solid = SOLID_NOT;
		pev->aiment = pev->owner;
		pev->body = 0;
		pev->frame = 0.0;
		pev->nextthink = gpGlobals->time + 0.1s;

		SET_MODEL(ENT(pev), "models/w_divinetitan.mdl");
		UTIL_SetSize(pev, Vector(-14, -4, 0), Vector(14, 3, 58));
		SetThink(&CDivineTitanBase::OnThink);

		}
		else if (m_iType == 2)
		{
			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_BBOX;
			pev->body = 0;
			pev->frame = 0.0;
			pev->gravity *= 1.5;
			pev->rendermode = kRenderTransAdd;
			pev->renderamt = 255;
			pev->nextthink = gpGlobals->time + 0.1s;

			SET_MODEL(ENT(pev), "models/divinetitan_missile.mdl");
			UTIL_SetSize(pev, Vector(-4, -4, -4), Vector(4, 4, 4));
			SetTouch(&CDivineTitanBase::OnTouch);

		}
	}
	void CDivineTitanBase::MakeLoopGenerator()
	{
		const float flRadius = GetRange_Instance();

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);
		if (!pAttackPlayer)
			return;


		phs9_10_11.clear();

		if (m_pFirstEnemy && !m_pFirstEnemy->IsAlive())
		{
			m_pFirstEnemy = nullptr;
			phs9_10_11.clear();
			MakeLoopGenerator();
		}

#ifndef CLIENT_DLL
		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pAttackPlayer->pev->origin, flRadius)) != nullptr)
		{
			if (!pAttackPlayer->IsAlive())
				continue;

#ifndef CLIENT_DLL
			if (g_pGameRules->PlayerRelationship(pAttackPlayer, pEntity) == GR_TEAMMATE)
				continue;
#endif

			if (pEntity->pev == pAttackPlayer->pev)
				continue;

			if (!pEntity->IsAlive())
				continue;


			if (!m_pFirstEnemy)
				m_pFirstEnemy = pEntity;

			EHANDLE eh;
			eh.Set(pEntity->edict());
			phs9_10_11.push_back(eh);


		}
		
		Vector pOrigin; 
		if (m_pFirstEnemy)
			pOrigin = m_pFirstEnemy->pev->origin;
		Vector mOrigin, mAngles;

		GET_ATTACHMENT(ENT(pAttackPlayer->pev), 0, mOrigin, mAngles);
		mOrigin.z += 30;
		if(m_pFirstEnemy && m_pFirstEnemy->IsAlive())
			CreateBeam(0, mOrigin, pOrigin, ENT(pAttackPlayer->pev));
		LoopInstantDamage(pAttackPlayer);
#endif
	}
	void CDivineTitanBase::LoopInstantDamage(CBasePlayer* pAttackPlayer)
	{
		size_t v8 = 0;
		CBaseEntity* m_iSaveEnemies[3]{};
		for (CBaseEntity* pEntity : phs9_10_11)
		{
			if (v8 >= 3)
				break;
			if (!pEntity)
				continue;
			if (!pEntity->IsAlive())
				continue;

			Vector vecDirection = (pEntity->pev->origin - pAttackPlayer->pev->origin).Normalize();

			TraceResult tr;
			UTIL_TraceLine(pAttackPlayer->pev->origin, pEntity->pev->origin, missile, ENT(pAttackPlayer->pev), &tr);

			ClearMultiDamage();
			pEntity->TraceAttack(pAttackPlayer->pev, GetDamage_Instance(), vecDirection, &tr, DMG_EXPLOSION);
			ApplyMultiDamage(pAttackPlayer->pev, pAttackPlayer->pev);
			
			if (v8 < 3)
			{
				m_iSaveEnemies[v8] = pEntity;
#ifndef CLIENT_DLL
				CreateBeam(0, m_iSaveEnemies[v8 ? v8 - 1 : 0]->pev->origin, m_iSaveEnemies[v8]->pev->origin, ENT(pAttackPlayer->pev));

				MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pEntity->pev->origin);
				WRITE_BYTE(TE_EXPLOSION);
				WRITE_COORD(pEntity->pev->origin.x);
				WRITE_COORD(pEntity->pev->origin.y);
				WRITE_COORD(pEntity->pev->origin.z);
				WRITE_SHORT(MODEL_INDEX("sprites/ef_divinetitan_amode_hit.spr"));
				WRITE_BYTE(8);
				WRITE_BYTE(20);
				WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
				MESSAGE_END();
#endif
				++v8;
			}
		}
	}
	void CDivineTitanBase::OnThink()
	{
		if (!m_iType)
		{
			pev->nextthink = gpGlobals->time + 0.05s;
			if (pev->frame < 15.0)
			{
				pev->frame += 1.0;
			}
			else
			{
				pev->frame = 0.0;
			}
			pev->renderamt -= 255.0;

			if (!pev->renderamt)
			{
				SetThink(NULL);
				UTIL_Remove(this);
			}
		}
		if (m_iType == 1)
		{
			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackPlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);
			if (pAttackPlayer)
			{
				if (pAttackPlayer->m_flPlayerBuffData == 1)
				{
					pev->nextthink = gpGlobals->time + 0.3s;
					if (!pAttackPlayer->IsAlive())
					{
						ClearEnt(1);
					}
					if (!pAttackPlayer->m_flPlayerBuffData)
					{
						ClearEnt(1);
					}
					MakeLoopGenerator();
				}
				else if (pAttackPlayer->m_flPlayerBuffData == 2)
				{
					pev->nextthink = gpGlobals->time + 0.625s;
#ifndef CLIENT_DLL
					UTIL_MakeVectors(pAttackPlayer->pev->v_angle + pAttackPlayer->pev->punchangle);
					Vector vecSrc = pAttackPlayer->GetGunPosition() + gpGlobals->v_forward * 10;
					vecSrc.z += 60;
					CDivineTitanBase* pEnt = CDivineTitanBase::Create(2, vecSrc, pAttackPlayer->pev->angles, ENT(pAttackPlayer->pev), m_iTeam, pAttackPlayer->m_flPlayerBuffData);

					if (pEnt)
					{
						Vector vecVelocity = gpGlobals->v_forward * 1200;
						vecVelocity.z -= 0.6 * 250;
						pEnt->pev->velocity = std::move(vecVelocity);
					}
#endif
				}
			}
		}
	}
	void CDivineTitanBase::OnTouch(CBaseEntity* pOther)
	{
		if (pev->owner == pOther->edict())
			return;

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackePlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

		if (pAttackePlayer &&
			pOther->pev->takedamage != DAMAGE_NO &&
			pOther->IsAlive()
			)
		{
			Vector vecDirection = (pOther->pev->origin - pev->origin).Normalize();

			TraceResult tr;
			UTIL_TraceLine(pev->origin, pOther->pev->origin, missile, ENT(pAttackePlayer->pev), &tr);
			tr.iHitgroup = HITGROUP_CHEST; // ...

			ClearMultiDamage();
			pOther->TraceAttack(pAttackePlayer->pev, GetDamage_Missile(1), vecDirection, &tr, DMG_BULLET);
			ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);
		}


		const float flRadius = GetRange_Missile();
		const float flDamage = GetDamage_Missile(0);
		const Vector vecSrc = pev->origin;
		entvars_t* const pevAttacker = VARS(pev->owner);
		entvars_t* const pevInflictor = this->pev;
		int bitsDamageType = DMG_BULLET;

		TraceResult tr;
		const float falloff = flRadius ? flDamage / flRadius : 1;
		const int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
		{
			if (pEntity->pev->takedamage != DAMAGE_NO)
			{
				if (bInWater && !pEntity->pev->waterlevel)
					continue;

				if (!bInWater && pEntity->pev->waterlevel == 3)
					continue;

				if (pEntity->IsBSPModel())
					continue;

				if (pEntity->pev == pevAttacker)
					continue;

				Vector vecSpot = pEntity->BodyTarget(vecSrc);
				UTIL_TraceLine(vecSrc, vecSpot, missile, ENT(pevInflictor), &tr);

				if (tr.flFraction == 1.0f || tr.pHit == pEntity->edict())
				{
					if (tr.fStartSolid)
					{
						tr.vecEndPos = vecSrc;
						tr.flFraction = 0;
					}
					float flAdjustedDamage = flDamage - (vecSrc - pEntity->pev->origin).Length() * falloff;
					flAdjustedDamage = Q_max(0, flAdjustedDamage);

					if (tr.flFraction == 1.0f)
					{
						pEntity->TakeDamage(pevInflictor, pevAttacker, flAdjustedDamage, bitsDamageType);
					}
					else
					{
						tr.iHitgroup = HITGROUP_CHEST;
						ClearMultiDamage();
						pEntity->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType);
						ApplyMultiDamage(pevInflictor, pevAttacker);
					}
				}
			}
		}
		EMIT_SOUND_DYN(this->edict(), CHAN_VOICE, "weapons/divinetitan_B_mode-2.wav", VOL_NORM, ATTN_NORM, 0, 94);

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(MODEL_INDEX("sprites/ef_divinetitan_bmode_explo.spr"));
		WRITE_BYTE(5);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/divinetitan_B_mode-2_exp.wav", VOL_NORM, ATTN_NORM, 0, 94);

		SetTouch(NULL);
		UTIL_Remove(this);


	}
	float CDivineTitanBase::GetDamage_Missile(int IsTouch) const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return IsTouch ? 500.0f : 1000.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return IsTouch ? 250.0f : 500.0f;
#endif
		return IsTouch ? 25.0f : 50.0f;
	}
	float CDivineTitanBase::GetRange_Missile() const
	{
		return 200.0f;
	}

	float CDivineTitanBase::GetDamage_Instance() const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 500.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 150.0f;
#endif
		return 10.0f;
	}
	float CDivineTitanBase::GetRange_Instance() const
	{
		return 300.0f;
	}

	
#endif
}
