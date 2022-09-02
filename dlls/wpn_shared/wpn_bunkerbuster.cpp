#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_bunkerbuster.h"

#include "monsters.h"
#include "gamerules.h"

#include <vector>

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(weapon_bunkerbuster, CBunkerBuster)

		void CBunkerBuster::Spawn(void)
	{
		Precache();
		m_iId = WEAPON_BUNKERBUSTER;
		SET_MODEL(ENT(pev), "models/w_bunkerbuster.mdl");

		pev->classname = MAKE_STRING("weapon_bunkerbuster");

		m_iClip = -1;
		m_iDefaultAmmo = 1;
		m_bDelayFire = false;

		FallInit();
	}

	void CBunkerBuster::Precache(void)
	{
		PRECACHE_MODEL("models/v_bunkerbuster.mdl");
		PRECACHE_MODEL("models/p_bunkerbuster.mdl");
		PRECACHE_MODEL("models/w_bunkerbuster.mdl");

		PRECACHE_SOUND("weapons/divinetitan_A_mode-1_start.wav");

		PRECACHE_MODEL("sprites/muzzleflash248.spr");
		m_usFireBunkerBuster = PRECACHE_EVENT(1, "events/bunkerbuster.sc");
	}

	int CBunkerBuster::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "bunkerbuster";
		p->iMaxAmmo1 = 1;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = WEAPON_NOCLIP;
		p->iSlot = 3;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_BUNKERBUSTER;
		p->iWeight = HEGRENADE_WEIGHT;
		p->iFlags = 0;


		return 1;
	}

	BOOL CBunkerBuster::Deploy(void)
	{
		m_fMaxSpeed = 260;

		//Reset Wpn State.
		if (m_pPlayer->m_iFOV != 90)
			m_pPlayer->m_iFOV = 90;

		m_pPlayer->m_bResumeZoom = false;
		m_pPlayer->m_iLastZoom = 90;

		m_fInReload = 0;
		m_flLastEventCheck = gpGlobals->time + 0.2s;

#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgFade, NULL, m_pPlayer->edict());
		WRITE_SHORT(0);
		WRITE_SHORT(0);
		WRITE_SHORT(0x0000);
		WRITE_BYTE(100);
		WRITE_BYTE(100);
		WRITE_BYTE(100);
		WRITE_BYTE(0);
		MESSAGE_END();
#endif

		return DefaultDeploy("models/v_bunkerbuster.mdl", "models/p_bunkerbuster.mdl", 3, "grenade", UseDecrement() != FALSE);
	}

	void CBunkerBuster::Holster(int skiplocal)
	{
		m_pPlayer->m_iFOV = 90;
		m_fInReload = 0;

		return CBasePlayerWeapon::Holster(skiplocal);
	}
	void CBunkerBuster::GetHitPoint(Vector hOrigin)
	{
		Vector fOrigin; Vector fvAngle; Vector fvOffset; Vector feOrigin;
		Vector fTemp;

		fOrigin = m_pPlayer->pev->origin;
		fvAngle = m_pPlayer->pev->v_angle;
		fvOffset = m_pPlayer->pev->view_ofs;

		Vector fvOrigin;
		fvOrigin = fOrigin + fvOffset;
		AngleVectors(fvAngle, feOrigin, fTemp, fTemp);

		VectorScale(feOrigin, 8192.0, feOrigin);
		feOrigin = fvOrigin + feOrigin;

		TraceResult tr;

		UTIL_TraceLine(fvOrigin, feOrigin, dont_ignore_monsters, pev->owner, &tr);

		hOrigin = gpGlobals->trace_endpos;
	}

	Vector CBunkerBuster::GetAimingOrg()
	{
		Vector VecReturn;
		Vector fOrigin; Vector fvAngle; Vector fvOffset;

		fOrigin = m_pPlayer->pev->origin;
		fvAngle = m_pPlayer->pev->v_angle;
		fvOffset = m_pPlayer->pev->view_ofs;

		fOrigin = fOrigin + fvOffset;
		UTIL_MakeVectors(fvAngle);

		fvAngle = gpGlobals->v_forward;
		//VectorScale(fvAngle, 8192.0, fvAngle);
		fvAngle = 8192 * fvAngle;
		fvAngle = fvAngle + fOrigin;

		TraceResult tr;
		UTIL_TraceLine(fOrigin, fvAngle, dont_ignore_monsters, pev->owner, &tr);

		VecReturn = tr.vecEndPos;
		return VecReturn;
	}

	Vector CBunkerBuster::Get_ShootPosition(CBaseEntity* pevAttacker, Vector Start)
	{
		Vector end;

		end = pevAttacker->pev->v_angle;
		UTIL_MakeVectors(end);
		end = gpGlobals->v_forward;
		end = end * 8192.0;
		end = Start + end;

		TraceResult tr;
		UTIL_TraceLine(Start, end, dont_ignore_monsters, pevAttacker->edict(), &tr);
		end = tr.vecEndPos;

		return end;
	}



	void CBunkerBuster::PrimaryAttack(void)
	{

		if (m_flCoolDownTime <= gpGlobals->time && m_flBunkerBusterCoolDown <= gpGlobals->time)
		{
			if (m_bSightMode)
			{
				if (!m_bStartedAiming)
				{
					m_bStartedAiming = true;
					m_iAimedState = 1;
					m_fAimedTime = gpGlobals->time + 3.0s;

					m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10.0s;


#ifndef CLIENT_DLL
					//UTIL_ScreenFade(pPlayer, Vector(255, 255, 255), fadeTime, fadeHold, alpha, 0);
					//UTIL_ScreenFade(m_pPlayer, Vector(255, 187, 0), 2s, 10s, 50, 0x0004);

					auto flTimeLeft = duration_t(BunkerBusterPersonalTime);
					auto flTimeLeft2 = duration_t(BunkerBusterGlobalTime);

					MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
					WRITE_BYTE(29);
					WRITE_BYTE((int)m_bStartedAiming);
					WRITE_COORD(-1.0);
					WRITE_COORD(-1.0);
					MESSAGE_END();

					MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
					WRITE_BYTE(30);
					WRITE_COORD(3.0);
					MESSAGE_END();
#endif


					m_vecStart = Get_ShootPosition(m_pPlayer, m_pPlayer->GetGunPosition());

#ifndef CLIENT_DLL
					CBunkerBusterTarget* pEnt = CBunkerBusterTarget::CreateTarget(m_vecStart, g_vecZero, pev->owner, this, true);
					if (pEnt)
					{
						DROP_TO_FLOOR(ENT(pEnt->pev));
					}
#endif


#ifndef CLIENT_DLL
					/*MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, m_pPlayer->pev);
					WRITE_SHORT(3);
					MESSAGE_END();
					*/
#endif
					EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/bunkerbuster_gauge.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

				}
				else
				{
					if (gpGlobals->time >= m_fAimedTime)
					{
						if (m_bStartedAiming)
						{

							m_bStartedAiming = false;
							m_fAimedTime = invalid_time_point;

							m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
							m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;

							SendWeaponAnim(2, UseDecrement() != FALSE);

							m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
							m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_bunkerbuster.mdl");

							//GetHitPoint(m_vecEnd);
							m_vecEnd = Get_ShootPosition(m_pPlayer, m_pPlayer->GetGunPosition());

#ifndef CLIENT_DLL
							CBunkerBusterTarget* pEnt = CBunkerBusterTarget::CreateTarget(m_vecEnd, g_vecZero, pev->owner, this);
							if (pEnt)
							{
								DROP_TO_FLOOR(ENT(pEnt->pev));
							}
#endif

							float flTargetDis = 0.0f;
							flTargetDis = (m_vecEnd - m_vecStart).Length() / BunkerBusterMissileCounts * 1.0;
							//BunkerBusterMissileCounts
							for (int i = 0; i < BunkerBusterMissileCounts; i++)
							{
								if (ceil((m_vecEnd - m_vecStart).Length() / flTargetDis) <= 1)
									break;

								if (m_vecEnd == m_vecStart)
									break;

								Vector vecDir;
								vecDir = m_vecEnd - m_vecStart;

								if (!vecDir.Length())
									break;

								vecDir = vecDir.Normalize();
								float flDistance = MIN(flTargetDis * i, (m_vecEnd - m_vecStart).Length());

								Vector vecTemp; Vector vecScale;
								vecTemp = vecDir;

								//VectorScale(vecTemp, flDistance, vecScale);
								vecScale = vecTemp * flDistance;
								Vector vecStartNew;
								vecStartNew = m_vecStart + vecScale;
#ifndef CLIENT_DLL
								CBunkerBusterTarget* pEnt = CBunkerBusterTarget::CreateTarget(vecStartNew, g_vecZero, pev->owner, this);
								if (pEnt)
								{
									DROP_TO_FLOOR(ENT(pEnt->pev));
								}
#endif
								if (flTargetDis <= 100.0)
									break;
							}

#ifndef CLIENT_DLL
							CBunkerBusterBase* pEntBase = CBunkerBusterBase::CreateBaseEnt(1, m_vecStart, g_vecZero, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);
							if (pEntBase)
							{
								Vector SzVecVelocity;

								Vector vecCurrentOrg;
								vecCurrentOrg = m_vecStart;
								vecCurrentOrg.z = pEntBase->GetMaxHeight(vecCurrentOrg);
								m_vecEnd.z = m_vecStart.z = vecCurrentOrg.z;

								SzVecVelocity = UTIL_GetSpeedVector(m_vecStart, m_vecEnd, BunkerBuster_PlaneSpeed);
								pEntBase->InitB52(g_vecZero, m_vecStart, m_vecEnd);

							}
#endif
							pev->nextthink = gpGlobals->time;
							SetThink(&CBunkerBuster::OnSoundThink);

							m_bSightMode = false;
							m_flCoolDownTime = gpGlobals->time + BunkerBusterPersonalTime;
							m_flBunkerBusterCoolDown = gpGlobals->time + BunkerBusterGlobalTime;
#ifndef CLIENT_DLL		
							auto flTimeLeft = duration_t(BunkerBusterPersonalTime);
							auto flTimeLeft2 = duration_t(BunkerBusterGlobalTime);

							MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
							WRITE_BYTE(29);
							WRITE_BYTE((int)m_bSightMode);
							WRITE_COORD((float)(BunkerBusterPersonalTime / 1s));
							WRITE_COORD((float)(BunkerBusterGlobalTime / 1s));
							MESSAGE_END();



							MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
							WRITE_BYTE(30);
							WRITE_COORD(0.0);
							MESSAGE_END();

							MESSAGE_BEGIN(MSG_ONE, gmsgFade, NULL, m_pPlayer->edict());
							WRITE_SHORT(0);
							WRITE_SHORT(0);
							WRITE_SHORT(0x0000);
							WRITE_BYTE(100);
							WRITE_BYTE(100);
							WRITE_BYTE(100);
							WRITE_BYTE(0);
							MESSAGE_END();
#endif
						}
					}
					else
					{


					}
				}
			}
		}

	}
	void CBunkerBuster::OnSoundThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.625s;
		if (m_iSoundCounts < 6)
		{
			m_iSoundCounts++;
			PLAYBACK_EVENT_FULL(FEV_HOSTONLY, ENT(m_pPlayer->pev), m_usFireBunkerBuster, 0.0, g_vecZero, g_vecZero, gpGlobals->v_forward.x, gpGlobals->v_forward.y, 0, 0, TRUE, FALSE);
		}
		else SetThink(NULL);


	}

	void CBunkerBuster::SecondaryAttack(void)
	{
		if (m_pPlayer->m_iFOV != 90)
		{
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
			m_bSightMode = 0;

			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_bunkerbuster.mdl");

			SendWeaponAnim(2, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		}
		else
		{
			m_bSightMode = true;
			m_flChangeView = gpGlobals->time + 0.36s;

			SendWeaponAnim(1, UseDecrement() != FALSE);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.36s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8s;

#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(29);
			WRITE_BYTE((int)m_bStartedAiming);
			WRITE_COORD(-1.0);
			WRITE_COORD(-1.0);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(30);
			WRITE_COORD(0.0);
			MESSAGE_END();

#endif
		}
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgFade, NULL, m_pPlayer->edict());
		WRITE_SHORT(0);
		WRITE_SHORT(0);
		WRITE_SHORT(0x0000);
		WRITE_BYTE(100);
		WRITE_BYTE(100);
		WRITE_BYTE(100);
		WRITE_BYTE(0);
		MESSAGE_END();
#endif
	}

	void CBunkerBuster::ItemPostFrame()
	{
		//Delay for changing fov and viewmodel.
		if (m_bSightMode)
		{
			if (m_flChangeView != invalid_time_point && m_flChangeView < gpGlobals->time)
			{
#ifndef CLIENT_DLL
				EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "items/equip_nvg.wav", VOL_NORM, ATTN_NORM);


				MESSAGE_BEGIN(MSG_ONE, gmsgFade, NULL, m_pPlayer->edict());
				WRITE_SHORT((1 << 12) * 2);
				WRITE_SHORT((1 << 10) * 10);
				WRITE_SHORT(0x0004);
				WRITE_BYTE(171);
				WRITE_BYTE(242);
				WRITE_BYTE(0);
				WRITE_BYTE(50);
				MESSAGE_END();

#endif
				m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 75;
				m_pPlayer->pev->viewmodel = MAKE_STRING("");

				m_flChangeView = invalid_time_point;
			}
		}

		return CBasePlayerWeapon::ItemPostFrame();
	}


	void CBunkerBuster::WeaponIdle(void)
	{
		if (m_bStartedAiming)
		{
			m_bStartedAiming = false;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;

			ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "Failed to set up target.");
#ifndef CLIENT_DLL
			/*
			MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, m_pPlayer->pev);
			WRITE_SHORT(0);
			MESSAGE_END();
			*/

			int player = m_pPlayer->entindex();

			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
			WRITE_BYTE(79);
			WRITE_SHORT(player);
			WRITE_SHORT(CHAN_WEAPON);
			WRITE_STRING("weapons/bunkerbuster_gauge.wav");
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(29);
			WRITE_BYTE((int)m_bStartedAiming);
			WRITE_COORD(0.0);
			WRITE_COORD(0.0);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
			WRITE_BYTE(30);
			WRITE_COORD(0.0);
			MESSAGE_END();


#endif

		}

		if (m_flTimeWeaponIdle <= UTIL_WeaponTimeBase())
		{
			SendWeaponAnim(0, UseDecrement() != FALSE);
		}
	}


#ifndef CLIENT_DLL
	LINK_ENTITY_TO_CLASS(bunkerbuster_target, CBunkerBusterTarget)

		CBunkerBusterTarget* CBunkerBusterTarget::CreateTarget(const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner, CBunkerBuster* pWeapon, bool IsSpecial)
	{
		edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("bunkerbuster_target"));

		if (FNullEnt(pent))
		{
			ALERT(at_console, "NULL Ent in Create!\n");
			return NULL;
		}

		CBunkerBusterTarget* pTar = (CBunkerBusterTarget*)Instance(pent);

		if (pTar)
		{
			if (IsSpecial)
			{
				pTar->m_bFastRemove = true;
			}

			pTar->pev->owner = pentOwner;
			pTar->pev->origin = vecOrigin;
			pTar->pev->angles = vecAngles;
			pTar->m_pWeapon = pWeapon;
			pTar->Spawn();

		}

		return pTar;
	}

	void CBunkerBusterTarget::Precache(void)
	{
		PRECACHE_MODEL("models/bunkerbuster_target.mdl");
		PRECACHE_SOUND("weapons/bunkerbuster_target_siren.wav");

	}

	void CBunkerBusterTarget::Spawn()
	{
		Precache();

		pev->movetype = MOVETYPE_TOSS;
		pev->solid = SOLID_NOT;
		pev->body = 0;

		//pev->light_level = 255;
		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 255.0;

		pev->frame = 0.0;
		pev->framerate = 1.0;

		pev->animtime = gpGlobals->time;
		pev->nextthink = gpGlobals->time + 0.1s;
		m_flRemoveTime = gpGlobals->time + GetLifeTime();

		SET_MODEL(ENT(pev), "models/bunkerbuster_target.mdl");

		SetThink(&CBunkerBusterTarget::OnThink);

	}
	void CBunkerBusterTarget::OnThink(void)
	{
		pev->nextthink = gpGlobals->time + 0.1s;
		if (m_flRemoveTime != invalid_time_point && m_flRemoveTime < gpGlobals->time)
		{
			SetThink(NULL);
			UTIL_Remove(this);
		}

		CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
		CBasePlayer* pOwnerPlayer = nullptr;
		if (pOwner && pOwner->IsPlayer())
			pOwnerPlayer = static_cast<CBasePlayer*>(pOwner);

		if (pOwnerPlayer)
		{
			if (m_bFastRemove)
			{
				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pOwnerPlayer->m_rgpPlayerItems[GRENADE_SLOT];

				if (pWeapon->m_iId == WEAPON_BUNKERBUSTER)
				{
					if (pWeapon)
					{
						if (!m_pWeapon->m_bStartedAiming)
						{
							UTIL_Remove(this);
							return;
						}
					}
				}
				else
				{
					UTIL_Remove(this);
					return;
				}

			}
		}
	}



#endif
}
