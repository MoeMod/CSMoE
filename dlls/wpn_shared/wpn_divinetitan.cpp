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
#include <engine/client/client.h>
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
		Precache();
		m_iId = WEAPON_DIVINETITAN;
		SET_MODEL(ENT(pev), "models/w_divinetitan.mdl");

		pev->classname = MAKE_STRING("weapon_divinetitan");

#ifndef CLIENT_DLL
		m_flTimeholster = gpGlobals->time;

		m_pTitan = NULL;

#endif

		m_iClip = -1;
		m_iDefaultAmmo = 50;

		m_bDelayFire = false;

		FallInit();
	}

	void CDivineTitan::Precache(void)
	{
		PRECACHE_MODEL("models/v_divinetitan.mdl");
		PRECACHE_MODEL("models/p_divinetitan.mdl");
		PRECACHE_MODEL("models/w_divinetitan.mdl");
		//PRECACHE_MODEL("sprites/bunkerbuster_fire.spr");
		

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
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
			m_iClip = WEAPON_NOCLIP;
			m_iDefaultAmmo = 0;

			return TRUE;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	int CDivineTitan::GetClip(void)
	{
		return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
	}

	void CDivineTitan::ChangeClip(int clip ,bool bSecond)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= clip;
		if(bSecond)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = clip;

	}

	BOOL CDivineTitan::Deploy(void)
	{

		m_fMaxSpeed = 260;
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
		case 2:
		{
			m_iDeployAnim = WEAPON_ANIM_DRAW3;
			break;
		}
		}
		m_bCanThrow = true;

#ifndef CLIENT_DLL
		if (m_flTimeholster != invalid_time_point)
		{
			if (!m_iWpnState)
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeholster) / 0.20s;
			}
				
			else
			{
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= (gpGlobals->time - m_flTimeholster) / 0.25s;
			}
		}
		else
		{
			//warning?! Invalid at all!!
			//m_iSaveShoots += MIN((gpGlobals->time - m_flTimeholster) / 0.25s,1);
		}

		if (m_iSaveShoots >= 70)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;
			m_iSaveShoots = 0;
		}

			
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 100)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 100;

		if (m_iWpnState)
		{
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 20)
				m_iFade = 1;
			else
				m_iFade = 0;

		}

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		{
			m_CrossHairState = 19;
		}
		else
			m_CrossHairState = 3;

		if (IsSignal())
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
			WRITE_BYTE(6);
			WRITE_BYTE(m_CrossHairState);
			MESSAGE_END();
		}
		else
		{
			m_CrossHairState = m_CrossHairStateStored = 3;
			MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
			WRITE_BYTE(6);
			WRITE_BYTE(m_CrossHairState);
			MESSAGE_END();
		}
		m_flTimeholster = gpGlobals->time;
		
#endif
		return DefaultDeploy("models/v_divinetitan.mdl", "models/p_divinetitan.mdl", m_iDeployAnim, "grenade", UseDecrement() != FALSE);
	}

	void CDivineTitan::Holster(int skiplocal)
	{
#ifndef CLIENT_DLL
		m_flTimeholster = gpGlobals->time;


		MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
		WRITE_BYTE(6);
		WRITE_BYTE(3);
		MESSAGE_END();
#endif
		return CBasePlayerWeapon::Holster(skiplocal);
	}
	BOOL CDivineTitan::IsSignal()
	{
#ifndef CLIENT_DLL
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]&& !Q_strcmp(STRING(m_pPlayer->m_pActiveItem->pev->classname), "weapon_divinetitan"))
		{
			m_CrossHairState = 19;
			return TRUE;
		}
		else if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] && !Q_strcmp(STRING(m_pPlayer->m_pActiveItem->pev->classname), "weapon_divinetitan") && m_iWpnState == 2)
		{
			m_CrossHairState = 21;
			return TRUE;
		}
		else if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] && !Q_strcmp(STRING(m_pPlayer->m_pActiveItem->pev->classname), "weapon_divinetitan") && m_iWpnState == 2)
		{
			m_CrossHairState = 20;
			return TRUE;
		}
		else
		{
			m_CrossHairState = 3;
			return FALSE;
		}
#endif
		return TRUE;
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
			CDivineTitanBase* pEnt = CDivineTitanBase::Create(3, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState,this);
			CDivineTitanBase* pTitanBase = CDivineTitanBase::Create(1, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState, this);
			CDivineTitanBase* pTitanFake = CDivineTitanBase::Create(-1, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState, this);
			m_pTitan = pTitanBase;	
#endif

			m_iWpnState = 1;
			this->pev->iuser1 = 1;

			break;
		}
		case 1:
		{
			m_iPrimaryAttackTime = WEAPON_TIMEA_OFF;
			m_iPrimaryAnimTime = WEAPON_ANIM_ATODEFAULT_TIME;
			m_iPrimaryAnim = WEAPON_ANIM_ATODEFAULT;

			m_iWpnState = 0;
			if (this->pev->iuser1)
				this->pev->iuser1 = 0;
			break;
		}
		case 2:
		{
			m_iPrimaryAttackTime = WEAPON_TIMEABSWITCH;
			m_iPrimaryAnimTime = WEAPON_ANIM_BTOA_TIME;
			m_iPrimaryAnim = WEAPON_ANIM_BTOA;
#ifndef CLIENT_DLL
			CDivineTitanBase* pEnt = CDivineTitanBase::Create(3, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState, this);
#endif

			m_iWpnState = 1;
			this->pev->iuser1 = 1;
			break;
		}
		default:
		{
			m_iPrimaryAttackTime = WEAPON_TIMEA_ON;
			m_iPrimaryAnimTime = WEAPON_ANIM_DEFAULTTOA_TIME;
			m_iPrimaryAnim = WEAPON_ANIM_DEFAULTTOA;

#ifndef CLIENT_DLL
			CDivineTitanBase* pEnt = CDivineTitanBase::Create(3, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState, this);
			CDivineTitanBase* pTitanBase = CDivineTitanBase::Create(1, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState, this);
			CDivineTitanBase* pTitanFake = CDivineTitanBase::Create(-1, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState, this);
#endif

			m_iWpnState = 1;
			this->pev->iuser1 = 1;

			break;

		}
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + m_iPrimaryAnimTime;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + m_iPrimaryAttackTime;
		//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + m_iPrimaryAttackTime;

		SendWeaponAnim(m_iPrimaryAnim, UseDecrement() != FALSE);

		m_pPlayer->m_flPlayerBuffData = m_iWpnState;

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
			CDivineTitanBase* pTitanBase = CDivineTitanBase::Create(1, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState, this);
			CDivineTitanBase* pTitanFake = CDivineTitanBase::Create(-1, m_pPlayer->pev->origin, m_pPlayer->pev->angles, ENT(m_pPlayer->pev), m_iTeam, m_iWpnState, this);
#endif

			m_iWpnState = 2;
			this->pev->iuser1 = 1;


			if (IsSignal())
			{
				//m_CrossHairStateStored = m_CrossHairState = 19;
#ifndef CLIENT_DLL
				MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
				WRITE_BYTE(6);
				WRITE_BYTE(m_CrossHairState);
				MESSAGE_END();
				m_CrossHairStateStored = m_CrossHairState;
#endif
			}


			break;
		}
		case 1:
		{
			m_iSecondaryAttackTime = WEAPON_TIMEABSWITCH;
			m_iSecondaryAnimTime = WEAPON_ANIM_ATOB_TIME;
			m_iSecondaryAnim = WEAPON_ANIM_ATOB;

			m_iWpnState = 2;
			this->pev->iuser1 = 1;

			if (IsSignal())
			{
				//m_CrossHairStateStored = m_CrossHairState = 19;
#ifndef CLIENT_DLL
				MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
				WRITE_BYTE(6);
				WRITE_BYTE(m_CrossHairState);
				MESSAGE_END();
				m_CrossHairStateStored = m_CrossHairState;
#endif
			}


			break;
		}
		default:
		{
			m_iSecondaryAttackTime = WEAPON_TIMEB_OFF;
			m_iSecondaryAnimTime = WEAPON_ANIM_BTODEFAULT_TIME;
			m_iSecondaryAnim = WEAPON_ANIM_BTODEFAULT;

			m_iWpnState = 0;
			this->pev->iuser1 = 0;
			break;
		}
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + m_iSecondaryAnimTime;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + m_iSecondaryAttackTime;
		//m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + m_iSecondaryAttackTime;

		SendWeaponAnim(m_iSecondaryAnim, UseDecrement() != FALSE);

		m_pPlayer->m_flPlayerBuffData = m_iWpnState;
	}
	void CDivineTitan::ItemPostFrame()
	{
		m_iButton = m_pPlayer->pev->button;

		//if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]||!m_iSaveSpecialAmmo)
		//{
		//	ResetSequence();
		//	return CBasePlayerWeapon::ItemPostFrame();
		//}

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;
			m_pPlayer->m_flPlayerBuffData = 0;
		}
		if (m_flBlockTime <= gpGlobals->time)
		{
			m_iBlockIdle = 0;
			m_flBlockTime = invalid_time_point;
		}
		if (m_flTimeNextMessage < gpGlobals->time)
		{
			if (m_CrossHairState != m_CrossHairStateStored)
			{
#ifndef CLIENT_DLL
				MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
				WRITE_BYTE(6);
				WRITE_BYTE(m_CrossHairState);
				MESSAGE_END();
#endif
			}

			m_flTimeNextMessage = gpGlobals->time + 0.1s;
			m_CrossHairStateStored = m_CrossHairState;
		}

#ifndef CLIENT_DLL
		if (!m_pPlayer->m_flPlayerBuffData)
		{
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 100)
			{
				if (m_flTimeNextCharge <= gpGlobals->time)
				{
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;
					m_flTimeNextCharge = gpGlobals->time + 0.20s;			
				}
			}
		}
		else
		{
//			if (m_flTimeNextCharge <= gpGlobals->time)
//			{
//				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
//				m_flTimeNextCharge = gpGlobals->time + 0.25s;
//
//				if (m_iSaveShoots < 70)
//				{
//					if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
//						m_iSaveShoots++;
//				}
//				else
//				{
//					m_iSaveShoots = 0;
//					m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;
//
//#ifndef CLIENT_DLL
//					if (IsSignal())
//					{
//						//m_CrossHairStateStored = m_CrossHairState = 19;
//
//						MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
//						WRITE_BYTE(6);
//						WRITE_BYTE(m_CrossHairState);
//						MESSAGE_END();
//						m_CrossHairStateStored = m_CrossHairState;
//					}
//					EMIT_SOUND_DYN(pev->owner, CHAN_WEAPON, "weapons/divinetitan_charge.wav", VOL_NORM, ATTN_NORM, 0, 94);
//#endif
//				}
//			}
		}

#endif


		if (m_iButton & IN_RELOAD && m_bCanThrow && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		{
			SendWeaponAnim(WEAPON_ANIM_PULLPIN, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.36s;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.36s;

			m_iAutoAttackStage = 1;
			m_bCanThrow = false;
		}

		if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		{
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

				m_pPlayer->m_flPlayerBuffData = 0.0;
				m_iWpnState = 0;
				m_iAutoAttackStage = 0;
				m_bCanThrow = true;
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;

#ifndef CLIENT_DLL
				if (IsSignal())
				{
					m_CrossHairState = m_CrossHairStateStored = 3;
					MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, m_pPlayer->pev);
					WRITE_BYTE(5);
					WRITE_BYTE(m_CrossHairState);
					MESSAGE_END();
				}
#endif
				break;
			}
			}
		}

		//Whenever if the clip is full or skills enabled, stop charging.
		return CBasePlayerWeapon::ItemPostFrame();
	}


	void CDivineTitan::WeaponIdle(void)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
		if(m_iBlockIdle)
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



	}
	void CDivineTitan::UpdateOnRemove(void)
	{

#ifndef CLIENT_DLL
		if (m_pTitan)
		{
			if (m_pTitan->m_pWeapon)
			{
				if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] || !m_iWpnState)
				{
					m_pTitan->m_pWeapon = NULL;
				}
			}
			//RemoveEntity();
		}

		CBaseEntity::UpdateOnRemove();
#endif
	}
	void CDivineTitan::RemoveEntity(void)
	{
#ifndef CLIENT_DLL
		if (m_pTitan)
		{
			if (m_pTitan->m_pWeapon == NULL)
			{
				m_pTitan->ClearEnt(1);
				m_pTitan = NULL;
			}
				
		}
#endif
	}
	void CDivineTitan::ResetSequence()
	{
		int m_iAnim;
		duration_t m_iNextAttackTime;
		duration_t m_iNextAnimTime;


		if (this->pev->sequence == 6 || m_iWpnState == 1)
		{
			m_iNextAttackTime = WEAPON_TIMEA_OFF;
			m_iNextAnimTime = WEAPON_ANIM_ATODEFAULT_TIME;
			m_iAnim = WEAPON_ANIM_ATODEFAULT;
		}
		if (this->pev->sequence == 10 || m_iWpnState == 2)
		{
			m_iNextAttackTime = WEAPON_TIMEB_OFF;
			m_iNextAnimTime = WEAPON_ANIM_BTODEFAULT_TIME;
			m_iAnim = WEAPON_ANIM_BTODEFAULT;
		}


		m_iBlockIdle = 1;
		m_flBlockTime = gpGlobals->time + m_iNextAnimTime;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + m_iNextAnimTime;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + m_iNextAttackTime;

		SendWeaponAnim(m_iAnim, UseDecrement() != FALSE);

		m_iWpnState = 0;

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

			pev->rendermode = kRenderTransAdd;
			pev->renderfx = kRenderFxGlowShell;

			MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, this->pev);
			WRITE_BYTE(16);
			WRITE_SHORT(this->entindex());
			WRITE_BYTE(17);
			MESSAGE_END();

			pev->rendercolor.x = 255;
			pev->rendercolor.y = 215;
			pev->rendercolor.z = 0;

			

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
								const float power = 800;
								vecOrigin = pev->origin;

								vecPullVelocity = (vecOrigin - player->pev->origin).Normalize() * power;
								player->pev->velocity = vecPullVelocity;

								if (vecDelta.Length() > 360)
									continue;

								player->TakeDamage(pev, pAttackPlayer->pev, GetDamage_SummonerAttack(1), DMG_BULLET | DMG_NEVERGIB);
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

								pEntity->TakeDamage(pev, pAttackPlayer->pev, GetDamage_SummonerAttack(1), DMG_BULLET | DMG_NEVERGIB);
							}
						}
					}
					pev->nextthink = gpGlobals->time + 0.2s;
				}
				else
				{

					pev->iuser1 = 2;
					m_flNextDamagePoint[0] = invalid_time_point;
					//fps 51 - 33 - 10???
					m_flNextDamagePoint[1] = gpGlobals->time;
					m_flNextDamagePoint[1] = gpGlobals->time + 0.125s;

					pev->nextthink = gpGlobals->time + 0.001s;
				}
			}
			if (pev->iuser1 == 2 && m_flNextDamagePoint[1] != invalid_time_point)
			{
				
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

								player->TakeDamage(this->pev, pAttackPlayer->pev, GetDamage_SummonerAttack(0), DMG_BULLET | DMG_NEVERGIB);

								if (player->m_bIsZombie && ~(player->m_iBuff) & BUFF_IGNORE_KNOCKBACK)
								{
									player->m_flVelocityModifier = 0.01;

									auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
									if (strategy)
									{
										strategy->SetStunSpeedTime(1.0s, 1.0f);
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

								pEntity->TakeDamage(this->pev, pAttackPlayer->pev, GetDamage_SummonerAttack(0), DMG_BULLET | DMG_NEVERGIB);
							}
						}
					}
					m_flNextDamagePoint[1] = invalid_time_point;
					SetThink(NULL);
					UTIL_Remove(this);
				}
				pev->nextthink = gpGlobals->time + 0.001s;
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
			WRITE_COORD(pev->origin.z + 65.0f);
			WRITE_SHORT(m_iEffect[0]);
			WRITE_BYTE(15);
			WRITE_BYTE(255);
			WRITE_BYTE(45);
			MESSAGE_END();
		}
		EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "weapons/divinetitan_exp.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

		MESSAGE_BEGIN(MSG_ALL, gmsgMPToCL, NULL, this->pev);
		WRITE_BYTE(16);
		WRITE_SHORT(this->entindex());
		WRITE_BYTE(17);
		MESSAGE_END();

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
		return 244.0f;
	}
	float CDivineTitanSummon::GetDamage_SummonerAttack(int IsAttraction) const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return IsAttraction ? 500.0f : 50000.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return IsAttraction ? 450.0f : 31600.0f;
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
		pev->effects |= EF_NODRAW; // 0x80u
		//Delete
		phs9_10_11.clear();
		m_pGroupList.clear();

		SetThink(nullptr);
		return UTIL_Remove(this);
	}
	CDivineTitanBase* CDivineTitanBase::Create(int iType, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, int iTeam, int iData, CDivineTitan* pWeapon)
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
			pEnt->m_pWeapon = pWeapon;
			

			pEnt->Spawn();
			
		}
		

		return pEnt;
	}

	void CDivineTitanBase::Precache(void)
	{
		PRECACHE_MODEL("sprites/ef_divinetitan_amode.spr");
		PRECACHE_MODEL("sprites/ef_divinetitan_amode_hit.spr");
		PRECACHE_MODEL("sprites/ef_divinetitan_bmode_explo.spr");
		PRECACHE_MODEL("sprites/smoke_thanatos9.spr");
		


		
		PRECACHE_MODEL("models/ef_divinetitan_amode.mdl");
		PRECACHE_MODEL("models/w_divinetitan.mdl");

		PRECACHE_SOUND("weapons/divinetitan_A_mode-1_loop.wav");
		PRECACHE_SOUND("weapons/divinetitan_B_mode-2_exp.wav");

		PRECACHE_SOUND("weapons/divinetitan_B_mode-2.wav");



	}

	void CDivineTitanBase::Spawn()
	{
		Precache();

		//Just a fake ent
		if (m_iType == -1)
		{
			pev->movetype = MOVETYPE_FLY;
			pev->solid = SOLID_NOT;
			pev->body = 0;
			pev->nextthink = gpGlobals->time + 0.1s;

			SET_MODEL(ENT(pev), "models/w_divinetitan.mdl");
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
			SetThink(&CDivineTitanBase::OnThink);
		}
		//Init For the beam.
		if (m_iType==0)
		{
			pev->flags = (pev->flags | FL_CUSTOMENTITY);
			pev->rendercolor.x = 255;
			pev->rendercolor.y = 255;
			pev->rendercolor.z = 255;

			pev->renderamt = 255.0f;
			pev->body = 0;
			pev->frame = 1.0;
			pev->animtime = invalid_time_point;
			pev->scale = 150.0;

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
			pev->animtime = gpGlobals->time + 20.0s;
			pev->nextthink = gpGlobals->time + 0.12048s ;
			
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

			pev->effects |= EF_NODRAW;
			
			SET_MODEL(ENT(pev), "models/w_divinetitan.mdl");
			UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
			//UTIL_SetSize(pev, Vector(-14, -4, 0), Vector(14, 3, 58));
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
		else if (m_iType == 3)
		{
			

			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_NOCLIP;
			pev->takedamage = DAMAGE_NO;
			SET_MODEL(ENT(pev), "models/ef_divinetitan_amode.mdl");

			pev->rendermode = kRenderTransTexture;
			pev->renderamt = 255.0;

			pev->frame = 0;
			pev->scale = 5.0;
			pev->sequence = 0;
			pev->fuser3 = 0.0;

			ResetSequenceInfo();

			SetThink(&CDivineTitanBase::OnThink);
			pev->animtime = gpGlobals->time + 0.01s;
			pev->nextthink = gpGlobals->time + 0.01s;
		}
	}
	void CDivineTitanBase::CheckDistanceBetweenEnemies()
	{
		
		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		static int iSaveIndex = NULL;

		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);
		if (!pAttackPlayer)
			return;
		const float flRadius = GetRange_Instance();
#ifndef CLIENT_DLL


		Vector vecDir = gpGlobals->v_forward;
		if (!phs9_10_11.empty())
		{
			CBaseEntity* pEnt = NULL;
			CBaseEntity* pRedirectEnt = NULL;
			float flMinDist = 360.0f;
			for (CBaseEntity* pEntity : phs9_10_11)
			{
				if (!pEntity)
					continue;

				if (std::find(m_pGroupList.begin(), m_pGroupList.end(), pEntity) != m_pGroupList.end())
					continue;

				Vector vecDirection = (pEntity->pev->origin - pAttackPlayer->pev->origin);

				float flDelta = vecDirection.Length();
				if (flDelta < flMinDist)
				{
					flMinDist = flDelta;
					vecDir = vecDirection.Normalize();
					pEnt = pEntity;
				}
			}
			if (pEnt)
			{

				phs9_10_11.erase(std::remove_if(phs9_10_11.begin(), phs9_10_11.end(), [=](CBaseEntity* pEntity) {
					if (pEntity)
					{
						float pEntityDistance = (pEntity->pev->origin - m_pFirstEnemy->pev->origin).Length();
						float pEntityDistance2 = (pEnt->pev->origin - m_pFirstEnemy->pev->origin).Length();
						if (pEntityDistance > pEntityDistance2)
						{
							iSaveIndex = pEnt->entindex();
							return true;
						}
					}

					return false;
					}), phs9_10_11.end());

				EHANDLE eh;
				eh.Set(pEnt->edict());
				m_pGroupList.push_back(eh);

				pRedirectEnt = UTIL_PlayerByIndex(iSaveIndex);

				if (pRedirectEnt)
				{
					EHANDLE eh;
					eh.Set(pRedirectEnt->edict());

					phs9_10_11.push_back(eh);
					pRedirectEnt = NULL;
				}
				pEnt = NULL;
				return;
			}
			else
			{
				m_pGroupList.clear();

				CheckDistanceBetweenEnemies();
				return;
				
			}
			
		}
#endif
	}

	void CDivineTitanBase::MakeLoopGeneratorAdvanced()
	{
		const float flRadius = GetRange_Instance();

		CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pAttackPlayer = nullptr;
		if (pAttacker && pAttacker->IsPlayer())
			pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);
		if (!pAttackPlayer)
			return;

		CBaseEntity* m_iTarget[3]{};
		float m_flDist[3]{};


		//First Enemy not alive.
		if (m_pFirstEnemy && !m_pFirstEnemy->IsAlive())
		{
			m_pFirstEnemy = nullptr;

			MakeLoopGeneratorAdvanced();
			return;
		}

		//First Enemy is alive but out of range.->Research the first enemy.
		if (m_pFirstEnemy && m_pFirstEnemy->IsAlive())
		{
			float pEntityDistance = (m_pFirstEnemy->pev->origin - pAttackPlayer->pev->origin).Length();
			if (pEntityDistance > flRadius)
			{
				m_pFirstEnemy = nullptr;

				MakeLoopGeneratorAdvanced();
				return;
			}
		}

#ifndef CLIENT_DLL
		CBaseEntity* pEntity = NULL;
		CBaseEntity* pStartEntity = NULL;
		pStartEntity = m_pFirstEnemy ? m_pFirstEnemy : pAttackPlayer;

		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pStartEntity->pev->origin, m_pFirstEnemy?263.39f:flRadius)) != nullptr)
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

			Vector vecTarget;
			vecTarget = pEntity->pev->origin;

			for (int i = 0; i < 3; i++)
			{
				float flDistance = (pStartEntity->pev->origin - pEntity->pev->origin).Length();
				if (!m_iTarget[i] || m_flDist[i] > flDistance)
				{
					if (i < 2)
					{
						for (int j = 1; j >= i; j--)
						{
							m_iTarget[j + 1] = m_iTarget[j];
							m_flDist[j + 1] = m_flDist[j];
						}
					}
					m_iTarget[i] = pEntity;
					m_flDist[i] = flDistance;
					m_pFirstEnemy = m_iTarget[0];
					break;
				}
			}
		}

		Vector pOrigin;
		if (m_pFirstEnemy)
			pOrigin = m_pFirstEnemy->pev->origin;
		Vector mOrigin, mAngles;

		GET_ATTACHMENT(ENT(pAttackPlayer->pev), 0, mOrigin, mAngles);
		mOrigin.z += 35;
		if (m_pFirstEnemy && m_pFirstEnemy->IsAlive())
			CreateBeam(0, mOrigin, pOrigin, ENT(pAttackPlayer->pev));

		for (int i = 0; i < 3; i++)
		{

			if (!m_iTarget[i])
				continue;
			if (!m_iTarget[i]->IsAlive())
				continue;

			Vector vecDirection = (m_iTarget[i]->pev->origin - pAttackPlayer->pev->origin).Normalize();

			TraceResult tr;
			UTIL_TraceLine(pAttackPlayer->pev->origin, m_iTarget[i]->pev->origin, missile, ENT(pAttackPlayer->pev), &tr);

			float flDamageAdjust = 1.0;
			if (i == 0)
				flDamageAdjust = 1.0;
			else if (i == 1)
				flDamageAdjust = 0.5;
			else flDamageAdjust = 0.25;

#ifndef CLIENT_DLL
			CreateBeam(0, m_iTarget[i - 1 < 0 ? 0 : i - 1]->pev->origin, m_iTarget[i]->pev->origin, ENT(pAttackPlayer->pev));

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, m_iTarget[i]->pev->origin);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(m_iTarget[i]->pev->origin.x);
			WRITE_COORD(m_iTarget[i]->pev->origin.y);
			WRITE_COORD(m_iTarget[i]->pev->origin.z);
			WRITE_SHORT(MODEL_INDEX("sprites/ef_divinetitan_amode_hit.spr"));
			WRITE_BYTE(8);
			WRITE_BYTE(20);
			WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
			MESSAGE_END();

			if (m_iTarget[i]->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)m_iTarget[i];

				if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
				{
					player->m_flVelocityModifier = 0.3;

					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(0.5s, 90.0f);
						strategy->SetStunGravityTime(0.5s);
					}
				}
			}
			
			//ClearMultiDamage();
			//m_iTarget[i]->TraceAttack(pAttackPlayer->pev, GetDamage_Instance() * flDamageAdjust, vecDirection, &tr, DMG_BULLET);
			//ApplyMultiDamage(pAttackPlayer->pev, pAttackPlayer->pev);
			
			m_iTarget[i]->TakeDamage(pev, pAttackPlayer->pev, GetDamage_Instance()* flDamageAdjust, DMG_BULLET);
#endif
		}
		if (m_iTarget[0]|| m_iTarget[1]|| m_iTarget[2])
		{
			if (m_flNextSoundTime < gpGlobals->time)
			{
				EMIT_SOUND_DYN(ENT(this->pev), CHAN_ITEM, "weapons/divinetitan_A_mode-1_loop.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
				m_flNextSoundTime = gpGlobals->time + 2.0s;
			}

		}
		else
		{
#ifndef CLIENT_DLL
			int player = pAttackPlayer->entindex();

			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, pAttackPlayer->edict());
			WRITE_BYTE(79);
			WRITE_SHORT(player);
			WRITE_SHORT(CHAN_ITEM);
			WRITE_STRING("weapons/divinetitan_A_mode-1_loop.wav");
			MESSAGE_END();
#endif
			m_flNextSoundTime = invalid_time_point;
		}
#endif
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

		//First Enemy not alive.
		if (m_pFirstEnemy && !m_pFirstEnemy->IsAlive())
		{
			m_pFirstEnemy = nullptr;
			phs9_10_11.clear();
			
			m_pGroupList.clear();
			MakeLoopGenerator();
			return;
		}
		//First Enemy is alive but out of range.->Research the first enemy.
		if (m_pFirstEnemy && m_pFirstEnemy->IsAlive())
		{
			float pEntityDistance = (m_pFirstEnemy->pev->origin - pAttackPlayer->pev->origin).Length();
			if (pEntityDistance > flRadius)
			{
				m_pFirstEnemy = nullptr;
				phs9_10_11.clear();

				MakeLoopGenerator();
				return;
			}
		}
		//Delete the elements which were incompatible with the owner at all.
		phs9_10_11.erase(std::remove_if(phs9_10_11.begin(), phs9_10_11.end(), [=](CBaseEntity* pEntity) {

			if (!pEntity->IsAlive() || bIsWallBetweenEntity(pEntity, pAttackPlayer))
				return true;
			//Recheck the distance if the enemies are out of range.
			//If true ,delete him.
			if (m_pFirstEnemy)
			{
				float pEntityDistance = (m_pFirstEnemy->pev->origin - pAttackPlayer->pev->origin).Length();
				if (pEntityDistance > flRadius)
					return true;
				//float pEntityDistance2 = (pEntity->pev->origin - pAttackPlayer->pev->origin).Length();
				//if (pEntityDistance2 > flRadius)
				//	return true;
			}

			return false;
			}), phs9_10_11.end());

#ifndef CLIENT_DLL
		CBaseEntity* pEntity = NULL;
		CBaseEntity* pStartEntity = NULL;
		pStartEntity = m_pFirstEnemy ? m_pFirstEnemy : pAttackPlayer;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pStartEntity->pev->origin, flRadius)) != nullptr)
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

			if (std::find(phs9_10_11.begin(), phs9_10_11.end(), eh) != phs9_10_11.end())
				continue;
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

			//ClearMultiDamage();
			//pEntity->TraceAttack(pAttackPlayer->pev, GetDamage_Instance(), vecDirection, &tr, DMG_EXPLOSION);
			//ApplyMultiDamage(pAttackPlayer->pev, pAttackPlayer->pev);
			
			if (v8 < 3)
			{
				float flDamageAdjust= 1.0;
				if(v8 == 0)
					flDamageAdjust = 1.0;
				else if(v8 == 1)
					flDamageAdjust = 0.5;
				else flDamageAdjust = 0.25;

				m_iSaveEnemies[v8] = pEntity;
#ifndef CLIENT_DLL
				CreateBeam(0, m_iSaveEnemies[v8-1<0?0:v8-1]->pev->origin, m_iSaveEnemies[v8]->pev->origin, ENT(pAttackPlayer->pev));

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

				if (pEntity->IsPlayer())
				{
					CBasePlayer* player = (CBasePlayer*)pEntity;

					if (player->m_bIsZombie  && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
					{
						player->m_flVelocityModifier = 0.3;

						auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
						if (strategy)
						{
							strategy->SetStunSpeedTime(0.5s, 90.0f);
							strategy->SetStunGravityTime(0.5s);
						}
					}
				}
				pEntity->TakeDamage(pev, pAttackPlayer->pev, GetDamage_Instance() * flDamageAdjust, DMG_BULLET);
				UTIL_BloodDrips(pEntity->pev->origin, vecDirection, 255, (int)GetDamage_Instance()* flDamageAdjust / (int)10.0);
#endif
				++v8;
			}
		}
	}
	void CDivineTitanBase::OnThink()
	{
		CBasePlayer* pAttack = dynamic_cast<CBasePlayer*>(CBasePlayer::Instance(pev->owner));
		if (pAttack)
		{
			if (pAttack->m_flPlayerBuffData)
			{

					CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pAttack->m_rgpPlayerItems[GRENADE_SLOT];
					if (!pWeapon)
					{
						pAttack->m_flPlayerBuffData = 0;
						this->m_pWeapon->m_flTimeholster = gpGlobals->time;
						UTIL_Remove(this);
						return;
					}

					if (pWeapon->m_iId != WEAPON_DIVINETITAN)
					{
						pAttack->m_flPlayerBuffData = 0;
						this->m_pWeapon->m_flTimeholster = gpGlobals->time;
						UTIL_Remove(this);
						return;
					}
				
				if (m_iType == 1  && this->m_pWeapon)
				{
					//Redirect timepoint

					if (m_flTimeNextCharge <= gpGlobals->time)
					{
						this->m_pWeapon->m_flTimeholster = gpGlobals->time;
						this->m_pWeapon->m_flTimeholster = invalid_time_point;

						m_flTimeNextCharge = gpGlobals->time + 0.20s;

						this->m_pWeapon->ChangeClip(1,false);
						this->m_pWeapon->m_iSaveShoots++;

						if (this->m_pWeapon->m_iSaveShoots == 70)
						{
							MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, pAttack->pev);
							WRITE_BYTE(15);
							WRITE_BYTE(80);
							MESSAGE_END();
						}

						if (this->m_pWeapon->m_iSaveShoots >= 70)
						{
							
							this->m_pWeapon->m_iSaveShoots = 0;
							this->m_pWeapon->ChangeClip(1, true);
							
							#ifndef CLIENT_DLL
							if (this->m_pWeapon->IsSignal())
							{
							
								MESSAGE_BEGIN(MSG_ONE, gmsgSpecialCrossHair, NULL, pAttack->pev);
								WRITE_BYTE(6);
								WRITE_BYTE(this->m_pWeapon->m_CrossHairState);
								MESSAGE_END();
								this->m_pWeapon->m_CrossHairStateStored = this->m_pWeapon->m_CrossHairState;
							}
							#endif
						}

					}
					if (!this->m_pWeapon->GetClip())
					{
						pAttack->m_flPlayerBuffData = 0;
						this->m_pWeapon->m_iFade = 0;			
						//ReStart charging only if the ammos run out.
						this->m_pWeapon->m_flTimeholster = gpGlobals->time;
						this->m_pWeapon->ResetSequence();
						ClearEnt(1);
						
					}
				}
			}
		}

		if (m_iType == -1)
		{
			pev->nextthink = gpGlobals->time + 0.01s;

			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackPlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);


			if (pAttackPlayer)
			{
				UTIL_SetOrigin(pev, pAttackPlayer->pev->origin + Vector(0, 0, 60));

				pev->angles = pAttackPlayer->pev->angles;
				pev->angles.x = 0;

			}
			if (pAttackPlayer && (!pAttackPlayer->IsAlive() || pAttackPlayer->m_bIsZombie))
			{
				pev->nextthink = invalid_time_point;
				pAttackPlayer->m_flPlayerBuffData = 0.0;

				ClearEnt(1);
			}
			if (!pAttackPlayer->m_flPlayerBuffData)
			{
				ClearEnt(1);
			}

		}
		if (m_iType == 0)
		{
			pev->nextthink = gpGlobals->time + 0.005s;
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

			pev->nextthink = gpGlobals->time + 0.01s;

			if (pAttackPlayer && (!pAttackPlayer->IsAlive() || pAttackPlayer->m_bIsZombie))
			{
				pev->nextthink = invalid_time_point;
				pAttackPlayer->m_flPlayerBuffData = 0.0;

				ClearEnt(1);
			}

			if (pAttackPlayer)
			{
				
				if (pAttackPlayer->m_flPlayerBuffData == 1)
				{
					//0.2s 
					pev->nextthink = gpGlobals->time + 0.12048s;
					//MakeLoopGenerator();
					MakeLoopGeneratorAdvanced();
				}
				else if (pAttackPlayer->m_flPlayerBuffData == 2)
				{
					pev->nextthink = gpGlobals->time + 0.6993s;
#ifndef CLIENT_DLL
					UTIL_MakeVectors(pAttackPlayer->pev->v_angle + pAttackPlayer->pev->punchangle);
					Vector vecSrc = pAttackPlayer->GetGunPosition() + gpGlobals->v_forward * 10;
					vecSrc.z += 60;
					CDivineTitanBase* pEnt = CDivineTitanBase::Create(2, vecSrc, pAttackPlayer->pev->angles, ENT(pAttackPlayer->pev), m_iTeam, pAttackPlayer->m_flPlayerBuffData,NULL);

					if (pEnt)
					{
						Vector vecVelocity = gpGlobals->v_forward * 1200;
						vecVelocity.z -= 0.6 * 250;
						pEnt->pev->velocity = std::move(vecVelocity);
					}
#endif
				}
				else
				{
					pAttackPlayer->m_flPlayerBuffData = 0.0;
					ClearEnt(1);
				}
			}
		}
		if (m_iType == 3)
		{
			pev->nextthink = gpGlobals->time + 0.05s;

			CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
			CBasePlayer* pAttackPlayer = nullptr;
			if (pAttacker && pAttacker->IsPlayer())
				pAttackPlayer = static_cast<CBasePlayer*>(pAttacker);

			if (this->m_pWeapon && this->m_pWeapon->GetClip() <= 20)
			{
				this->m_pWeapon->m_iFade = 1;
				if (m_iType == 3)
				{
					if (this->m_pWeapon)
					{
						if (this->m_pWeapon->GetClip() <= 20)
						{
							if (m_flNextFadeTime <= gpGlobals->time)
							{
								float flAmt; flAmt = pev->renderamt;
								if (flAmt <= 0)
								{
									flAmt += 255.0f;
								}
								else
								{
									if (flAmt)
										flAmt -= 255.0f;
								}
								pev->renderamt = flAmt;

								m_flNextFadeTime = gpGlobals->time + 0.25s;
							}
						}
					}
				}
			}


			if (pAttackPlayer)
			{
				UTIL_SetOrigin(pev, pAttackPlayer->pev->origin + Vector(0, 0, (FBitSet(pAttackPlayer->pev->flags, FL_DUCKING))?-13:-28));

				pev->angles = pAttackPlayer->pev->angles;
				pev->angles.x = 0;
				
			}
			if (pAttackPlayer && pAttackPlayer->m_flPlayerBuffData!=1)
			{
				ClearEnt(1);
			}

			if (pAttackPlayer && (!pAttackPlayer->IsAlive() || pAttackPlayer->m_bIsZombie))
			{
				ClearEnt(1);
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

			//ClearMultiDamage();
			//pOther->TraceAttack(pAttackePlayer->pev, GetDamage_Missile(1), vecDirection, &tr, DMG_BULLET);
			//ApplyMultiDamage(pAttackePlayer->pev, pAttackePlayer->pev);

			pOther->TakeDamage(pev, pAttackePlayer->pev, GetDamage_Missile(1), DMG_BULLET);

			if (pOther->IsPlayer())
			{
				CBasePlayer* player = (CBasePlayer*)pOther;

				if (player->m_bIsZombie && ~(player->m_iBuff) & BUFF_IGNORE_KNOCKBACK)
				{
					player->m_flVelocityModifier = 0.4;

					auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
					if (strategy)
					{
						strategy->SetStunSpeedTime(1.0s);
						strategy->SetStunGravityTime(1.0s);
					}
				}
			}

			EMIT_SOUND_DYN(pOther->edict(), CHAN_VOICE, "weapons/divinetitan_B_mode-2.wav", VOL_NORM, ATTN_NORM, 0, 94);
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

					if (pOther->IsPlayer())
					{
						CBasePlayer* player = (CBasePlayer*)pEntity;

						if (player->m_bIsZombie && ~(player->m_iBuff) & BUFF_IGNORE_KNOCKBACK)
						{
							player->m_flVelocityModifier = 0.4;

							auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
							if (strategy)
							{
								strategy->SetStunSpeedTime(1.0s);
								strategy->SetStunGravityTime(1.0s);
							}
						}
					}
				}
			}
		}
		

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(MODEL_INDEX("sprites/ef_divinetitan_bmode_explo.spr"));
		WRITE_BYTE(8);
		WRITE_BYTE(30);
		WRITE_BYTE(TE_EXPLFLAG_NODLIGHTS | TE_EXPLFLAG_NOPARTICLES | TE_EXPLFLAG_NOSOUND);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_EXPLOSION);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(MODEL_INDEX("sprites/smoke_thanatos9.spr"));
		WRITE_BYTE(10);
		WRITE_BYTE(15);
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
			return IsTouch ? 2500.0f : 3000.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return IsTouch ? 2000.0f : 500.0f;
#endif
		return IsTouch ? 25.0f : 50.0f;
	}
	float CDivineTitanBase::GetRange_Missile() const
	{
		return 100.0f;
	}

	float CDivineTitanBase::GetDamage_Instance() const
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 500.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 300.0f;
#endif
		return 10.0f;
	}
	float CDivineTitanBase::GetRange_Instance() const
	{
		return 326.8f;
	}

	
#endif
}
