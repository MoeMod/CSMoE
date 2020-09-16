/*
wpn_anaconda.cpp
Copyright (C) 2020 Moemod Haoyuan

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
#include "weapons/RadiusDamage.hpp"
#include "weapons/KnifeAttack.h"

#include <array>

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

using WeaponTemplate::Varibles::A;
using WeaponTemplate::Varibles::T;

class CGunkata : public LinkWeaponTemplate< CGunkata,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_USP>::template type,
		TPrecacheEvent,
		TReloadDefault,
		TPrimaryAttackRifle,
		TRecoilPunch,
		TWeaponIdleDefault
	>
{
public:
	static constexpr const auto &DefaultReloadTime = 2.3s;
	static constexpr const auto &DefaultDeployTime = 0.75s;
	static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
	static constexpr const char *V_Model = "models/v_gunkata.mdl";
	static constexpr const char *P_Model = "models/p_gunkata.mdl";
	static constexpr const char *W_Model = "models/w_gunkata.mdl";
	static constexpr const char *EventFile = "events/gunkata.sc";
	static constexpr const char *ClassName = "weapon_gunkata";
	static constexpr const char *AnimExtension = "dualpistols";
	static constexpr int MaxClip = 36;
	static constexpr float ArmorRatioModifier = 1.7f;
	static constexpr float AccuracyDefault = 1.1f;
	static constexpr const auto & AccuracyCalc = A - (0.3 - T) * 0.2;
	static constexpr float AccuracyMin = 0.4;
	static constexpr float AccuracyMax = 1.1;
	enum
	{
		ANIM_IDLE = 0, //6.0
		ANIM_IDLE2, //6.0
		ANIM_SHOOT1, //0.5
		ANIM_SHOOT1_EMPTY, //0.5
		ANIM_SHOOT2, //0.7
		ANIM_SHOOT2_EMPTY, //0.5
		ANIM_RELOAD, //2.0
		ANIM_RELOAD2, //2.0
		ANIM_DRAW1, //1.0
		ANIM_DRAW2, //1.0
		ANIM_SKILL1, //0.7
		ANIM_SKILL2, //0.7
		ANIM_SKILL3, //1.0
		ANIM_SKILL4, //1.0
		ANIM_SKILL5, //1.0
		ANIM_SKILL_LAST //1.0
	};
	static constexpr const auto & SpreadCalcNotOnGround = 0.15 * (1 - A);
	static constexpr const auto & SpreadCalcWalking = 0.12 * (1 - A);
	static constexpr const auto & SpreadCalcDucking = 0.1 * (1 - A);
	static constexpr const auto & SpreadCalcDefault = 0.11 * (1 - A);
	static constexpr const auto &CycleTime = 0.0825s;

	static constexpr const auto &WeaponIdleTime = 6.03s;
	static constexpr float RangeModifier = 0.95;
	static constexpr auto BulletType = BULLET_PLAYER_45ACP;
	static constexpr int Penetration = 8; // ?
	KnockbackData KnockBack = { 0, 0, 0, 0, 0.5f };
	std::array<float, 3> RecoilPunchAngleDelta = { -0.33, 0, 0 };

	enum GunkataMode{
		GUNKATA_MODE_A = 0,
		GUNKATA_MODE_B = 1,
		GUNKATA_MODE_B2 = 2,
		GUNKATA_MODE_B3 = 3,
		GUNKATA_MODE_B4 = 4,
	};
	static float GetDamage(GunkataMode iMode = GUNKATA_MODE_A)
	{
		if(iMode)
		{
			if(iMode == GUNKATA_MODE_A)
			{
#ifndef CLIENT_DLL
				if(g_pModRunning->DamageTrack() == DT_ZB)
				{
					return 87.0;
				}
				else if(g_pModRunning->DamageTrack() == DT_ZBS)
				{
					return 239.0;
				}
				else
#endif
				{
					return 22.0;
				}
			}
			else if(iMode == GUNKATA_MODE_B)
			{
#ifndef CLIENT_DLL
				if(g_pModRunning->DamageTrack() == DT_ZB)
				{
					return 95.0;
				}
				else if(g_pModRunning->DamageTrack() == DT_ZBS)
				{
					return 265.0;
				}
				else
#endif
				{
					return 30.0;
				}
			}
			else
			{
				return 0.0;
			}
		}
		else
		{
#ifndef CLIENT_DLL
			if(g_pModRunning->DamageTrack() == DT_ZB)
			{
				return 122.0;
			}
			else if(g_pModRunning->DamageTrack() == DT_ZBS)
			{
				return 240.0;
			}
			else
#endif
			{
				return 34.0;
			}
		}
		return 0.0;
	}

	BOOL Deploy(void) override
	{
		m_flAccuracy = AccuracyDefault;

#ifndef CLIENT_DLL
		m_iMode_pev_iuser1 = 0; // (this + 292) = 0
		m_flNextSpecialAttack1_pev_fuser1 = time_point_t::max(); // 0x7F7FFFFF; // 320
		m_flNextSpecialAttack5_pev_teleport_time = time_point_t::max(); // 0x7F7FFFFF; // 336
		m_flNextSpecialAttack9_pev_dmg_save = time_point_t::max(); // 0x7F7FFFFF; // 352
		m_flNextSpecialAttack11_pev_speed = time_point_t::max(); // 0x7F7FFFFF; // 360
#endif

		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		m_pPlayer->m_bShieldDrawn = false;
		m_fMaxSpeed = 250;
		int iDeployAnim = GetANIM_DRAW();
		BOOL ret = DefaultDeploy(V_Model, P_Model, iDeployAnim, AnimExtension, UseDecrement() != FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;
		return iDeployAnim;
	}

	int GetANIM_DRAW() const { return GetLRMode() ? ANIM_DRAW1:ANIM_DRAW2; }

	int GetANIM_IDLE1() const { return GetLRMode() ? ANIM_IDLE:ANIM_IDLE2; }

	PLAYER_ANIM GetPlayerAttackAnimation() const { return !GetLRMode() ? PLAYER_ATTACK1 : PLAYER_ATTACK2; }

	bool GetLRMode() const // bool sub_1026F9F0(this)
	{
		return !(((MaxClip - m_iClip) / 3) % 2);
	}

	void SecondaryAttack() override // int __usercall sub_1026EED0@<eax>(int a1@<ecx>, double a2@<st0>)
	{
#ifndef CLIENT_DLL
		if(m_iMode_pev_iuser1 != 1)
		{
			if(m_iClip > 0)
			{
				m_iMode_pev_iuser1 = 1;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = duration_t::max(); // 0x7F7FFFFF;
			}

		}
#endif
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				flCycleTime = 0.2s;
			}
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;

			return;
		}

		int iShootAnim = 0;
		if(!GetLRMode())
		{
			iShootAnim = 4;
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
		}
		else
		{

			iShootAnim = 2;
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.53s;
		}

		m_iShotsFired++;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy = df::AccuracyCalc::Get(*this)(A = m_flAccuracy)(T = ((gpGlobals->time - m_flLastFire) / 1s));
			CheckAccuracyBoundary();
		}
		m_flLastFire = get_gametime();

		--m_iClip;

		if(!((MaxClip - m_iClip) % 3))
		{
			//flCycleTime =  c_flAttackInterval[iBteWpn][1];
			flCycleTime = 0.31s; // this+148
			m_pPlayer->m_flNextAttack = 0.31s;
			m_flAccuracy = AccuracyDefault; //*(_DWORD *)(this + 228) = 1.1;
			++iShootAnim; //++v8;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.53s; // this+156  //v9 = sub_105E0FB0() + 0.52999997; *(float *)(this + 156) = v9;
		}

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, df::Distance::Get(*this), df::Penetration::Get(*this), df::BulletType::Get(*this), GetDamage(), df::RangeModifier::Get(*this), m_pPlayer->pev, df::ItemSlot::Get(*this) == PISTOL_SLOT, m_pPlayer->random_seed);

		int flags = 0;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), iShootAnim, FALSE);
		SendWeaponAnim(iShootAnim, UseDecrement() != FALSE);

		Recoil();
	}

	void Precache() override
	{
		//v1 = this;
		PRECACHE_MODEL("models/p_gunkata2.mdl");
		/*
        PRECACHE_MODEL("models/ef_gunkata.mdl");
        PRECACHE_MODEL("models/ef_gunkata_man.mdl");
        PRECACHE_MODEL("models/ef_gunkata_woman.mdl");
        PRECACHE_MODEL("models/ef_scorpion_hole.mdl");
        PRECACHE_SOUND("weapons/gunkata-1.wav");
        PRECACHE_SOUND("weapons/gunkata_idle.wav");
        PRECACHE_SOUND("weapons/gunkata_skill_01.wav");
        PRECACHE_SOUND("weapons/gunkata_skill_02.wav");
        PRECACHE_SOUND("weapons/gunkata_skill_03.wav");
        PRECACHE_SOUND("weapons/gunkata_skill_04.wav");
        PRECACHE_SOUND("weapons/gunkata_skill_05.wav");
        PRECACHE_SOUND("weapons/gunkata_skill_last_exp.wav");
        */
		//m_iShell = PRECACHE_MODEL("models/pshell.mdl");
		//*((_WORD *)v1 + 142) = precache_event(1, "events/gunkata.sc");
		//result = PRECACHE_EVENT(1, "events/gunkata_effect.sc");
		//*((_WORD *)v1 + 143) = result;
		//return result;

		PRECACHE_SOUND("weapons/turbulent9_hit1.wav");
		PRECACHE_SOUND("weapons/turbulent9_hit2.wav");
		return Base::Precache();
	}

	void ItemPostFrame() override
	{
#ifndef CLIENT_DLL
		switch(m_iMode_pev_iuser1)
		{
			case GUNKATA_MODE_B:
			{
				m_flNextSpecialAttack1_pev_fuser1 = get_gametime();
				m_flNextSpecialAttack2_pev_fuser2 = get_gametime();
				m_flNextSpecialAttack3_pev_fuser3 = get_gametime();
				m_flNextSpecialAttack4_pev_fuser4 = get_gametime();
				m_flNextSpecialAttack5_pev_teleport_time = get_gametime();
				m_flNextSpecialAttack6_pev_air_finished = get_gametime();
				m_flNextSpecialAttack7_pev_pain_finished = get_gametime();
				m_flNextSpecialAttack8_pev_dmg_take = get_gametime();
				m_flNextSpecialAttack9_pev_dmg_save = get_gametime();
				m_flNextSpecialAttack10_pev_dmgtime = get_gametime();
				m_flNextSpecialAttack11_pev_speed = get_gametime();

				m_flNextSpecialAttack4_pev_fuser4 = get_gametime() + 0.2s;
				m_flNextSpecialAttack2_pev_fuser2 = get_gametime() + 0.2s;

				m_iMode_pev_iuser1 = GUNKATA_MODE_B2;
				m_iAnim1_pev_iuser3 = 13;
				m_iAnim2_pev_iuser4 = 11;
				break;
			}
			case GUNKATA_MODE_B2:
			{
				if(m_iClip > 0)
				{
					if(m_pPlayer->pev->button & IN_ATTACK2)
					{
						if(get_gametime() > m_flNextSpecialAttack2_pev_fuser2)
						{
							float v14 = 1.0f;
							if(m_iAnim1_pev_iuser3 == 10 || m_iAnim1_pev_iuser3 == 11 || m_iAnim1_pev_iuser3 == 4)
							{
								v14 = 0.7;
							}
							else if(m_iAnim1_pev_iuser3 == 2)
							{
								v14 = 0.53;
							}
							// dword_108F7A0C(32,*(_DWORD *)(pPlayer + 540),*(_WORD *)(this + 286),0,pPlayer + 8,&qword_108F77EC,
							//	v14,0,iAnim1,0,1,1
							PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, v14, 0.0, m_iAnim2_pev_iuser4, 0, 1, 1); // show additional vmodel
							SendWeaponAnim(m_iAnim1_pev_iuser3, UseDecrement() != FALSE);
							/*
							// CGunkata_sub_101710E0(&iAnim1, (int)&v45, iAnim1);
							if(iAnim1 == iAnim2) // ???
							{
								iAnim2 = iAnim1;
								set_pev(this, pev_iuser4, iAnim2);
								// CGunkata_sub_1026DA30(iAnim1, (int)v25, (int)v24, v23);
								// CGunkata_sub_1004B410((int)*v20, iAnim2);
							}
							*/
							m_iAnim1_pev_iuser3 = m_iAnim2_pev_iuser4;
							m_iAnim2_pev_iuser4 += 1;
							if(m_iAnim2_pev_iuser4 == 14)
							{
								m_iAnim2_pev_iuser4 -= 4; // ???
							}
							m_flNextSpecialAttack2_pev_fuser2 = get_gametime() + 0.4s;
						}

						if(get_gametime() > m_flNextSpecialAttack3_pev_fuser3)
						{
							float v22 = 1.0f;
							if(m_iAnim1_pev_iuser3 == 10 || m_iAnim1_pev_iuser3 == 11 || m_iAnim1_pev_iuser3 == 4)
							{
								v22 = 0.7;
							}
							else if(m_iAnim1_pev_iuser3 == 2)
							{
								v22 = 0.53;
							}
							// dword_108F7A0C(32,*(_DWORD *)(pPlayer + 540),*(_WORD *)(this + 286),0,pPlayer + 8,&qword_108F77EC,
							//	v22,0,iAnim1,0,0,1);
							PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, v22, 0.0, m_iAnim2_pev_iuser4, 0, 1, 1); // show additional vmodel
							SendWeaponAnim(m_iAnim1_pev_iuser3, UseDecrement() != FALSE);
							/*
							// CGunkata_sub_101710E0(&iAnim1, (int)&v45, iAnim1);
							if(iAnim1 == iAnim2) // ???
							{
								iAnim2 = iAnim1;
								set_pev(this, pev_iuser4, iAnim2);
								// CGunkata_sub_1026DA30(iAnim1, (int)v25, (int)v24, v23);
								// CGunkata_sub_1004B410((int)*v20, iAnim2);
							}
							*/
							m_iAnim1_pev_iuser3 = m_iAnim2_pev_iuser4;
							m_iAnim2_pev_iuser4 += 1;
							if(m_iAnim2_pev_iuser4 == 14)
							{
								m_iAnim2_pev_iuser4 -= 4; // ???
							}
							m_flNextSpecialAttack3_pev_fuser3 = get_gametime() + v22 * 1.0s;
						}

						if(get_gametime() > m_flNextSpecialAttack1_pev_fuser1)
						{
							//v26 = id;
							// CGunkata_sub_10114650(id, 2.03, pEntity); // reload ?
							m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_gunkata2.mdl");
							// pPlayer->pev->?(296) = pEntity;
							// pPlayer->pev->?(304) = 0;
							//sub_104BB200(pPlayer);
							m_flNextSpecialAttack1_pev_fuser1 = get_gametime() + 2.03s;
						}

						if(get_gametime() > m_flNextSpecialAttack5_pev_teleport_time)
						{
							// dword_108F7A0C(32,*(_DWORD *)(pPlayer + 540),*(_WORD *)(this + 286),0,pPlayer + 8,&qword_108F77EC,
							//	0.87, 0,0,0,0,2);
							PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.87, 0.0, 0, 0, 0, 2);
							m_flNextSpecialAttack5_pev_teleport_time = get_gametime() + 0.87s;
						}

						if(get_gametime() > m_flNextSpecialAttack4_pev_fuser4)
						{
							int v26 = 0;
							if(m_pPlayer->pev->flags & FL_DUCKING)
								v26 = 6;
                            m_iStatus2_pev_watertype = m_iAnim2_pev_iuser4 - 10;
							PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 10.0, 2.0, v26+m_iStatus2_pev_watertype, 4, 0, 4);
							m_flNextSpecialAttack4_pev_fuser4 = get_gametime() + 0.2s;
						}

						if(get_gametime() > m_flNextSpecialAttack6_pev_air_finished)
						{
							RadiusAttack1();
							m_flNextSpecialAttack6_pev_air_finished = get_gametime() + 0.082500003s;
						}

						if(get_gametime() > m_flNextSpecialAttack7_pev_pain_finished)
						{
							--m_iClip;
							m_flNextSpecialAttack7_pev_pain_finished = get_gametime() + 0.082500003s;
						}

						// break;
					}
					else // aka !(m_pPlayer->pev->button & IN_ATTACK2)
					{
						// dword_108F7A0C(32,*(_DWORD *)(pPlayer + 540),*(_WORD *)(this + 286),0,pPlayer + 8,&qword_108F77EC
						//	0 ,0, 0, 0, 0, 5);
						PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 5);
						m_iMode_pev_iuser1 = GUNKATA_MODE_B4;
						return CBasePlayerWeapon::ItemPostFrame();
					}
				}
				else // m_iClip <= 0
				{
					SendWeaponAnim(15, UseDecrement() != FALSE); //(*(void (__cdecl **)(signed int, _DWORD))(*(_DWORD *)a1 + 540))(15, 0);
					// v6 = sub_104BB1D0(id, "gunkata_end"); // lookup_sequence

					//set_pev(id, ???(pPlayer->pev+304), 0); // (pPlayer->pev) + 304) = 0;
					//*(_WORD *)(pPlayer + 2308) = v6;
					//*(float *)(pPlayer + 2304) = *(float *)gametime + 1.1;


					m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_gunkata.mdl");
					m_flNextSpecialAttack8_pev_dmg_take =  get_gametime() + 0.63s;
					m_flNextSpecialAttack9_pev_dmg_save =  get_gametime() + 0.63s;
					m_flNextSpecialAttack10_pev_dmgtime =  get_gametime() + 1.1s;
					m_flNextSpecialAttack11_pev_speed =  get_gametime() + 1.1s;


					// dword_108F7A0C(32,*(_DWORD *)(pPlayer + 540),*(_WORD *)(this + 286),0,pPlayer + 8,&qword_108F77EC,
					//	0 ,0, 0, 0, 0, 5);
					PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 5);
					m_iMode_pev_iuser1 = GUNKATA_MODE_B3;

					return CBasePlayerWeapon::ItemPostFrame();
				}
				break;
			}
			case GUNKATA_MODE_B3:
			{
				if(get_gametime() > m_flNextSpecialAttack8_pev_dmg_take)
				{
					// dword_108F7A0C(32,*(_DWORD *)(pPlayer + 540),*(_WORD *)(this + 286),0,pPlayer + 8,&qword_108F77EC,
					//	1.0 ,0.22, 0, 0, 0, 3);
					PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 1.0 ,0.22, 0, 0, 0, 3);
					m_flNextSpecialAttack8_pev_dmg_take = time_point_t::max(); // 0x7F7FFFFF

				}

				if(get_gametime() > m_flNextSpecialAttack9_pev_dmg_save)
				{
					RadiusAttack2();
					if(g_pModRunning->DamageTrack() == DT_NONE)
					{
						m_flNextSpecialAttack9_pev_dmg_save = time_point_t::max(); // 0x7F7FFFFF
					}
				}

				if(get_gametime() > m_flNextSpecialAttack10_pev_dmgtime)
				{
					// complete the reload.
					int delta = Q_min(MaxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

					// Add them to the clip
					m_iClip += delta;
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= delta;
					m_pPlayer->TabulateAmmo();
					m_fInReload = FALSE;

					m_flNextSpecialAttack10_pev_dmgtime = time_point_t::max(); // 0x7F7FFFFF
				}

				if(get_gametime() > m_flNextSpecialAttack11_pev_speed)
				{
					m_iMode_pev_iuser1 = GUNKATA_MODE_B4;
					return CBasePlayerWeapon::ItemPostFrame();
				}

				break;
			}
			case GUNKATA_MODE_B4:
			{

				m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_gunkata.mdl");
				// *(_DWORD *)(pPlayer + 2304) = -1.0;
				// *(_WORD *)(pPlayer + 2308) = -1;
				/*
                if ( (*(int (__stdcall **)(int, int))(pPlayer + 152))(a2, a3) )
                {
                    *(_DWORD *)(pPlayer + 104) = 1;
                    (*(void (__stdcall **)(_DWORD))(pPlayer + 448))(0);
                    *(_DWORD *)(pPlayer->pev + 304) = 0;
                    sub_104BB200(pPlayer);
                }
                */

				SendWeaponAnim(!GetLRMode() ? 9:8, UseDecrement() != FALSE);
				m_iMode_pev_iuser1 = GUNKATA_MODE_A;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;
				break;
			}
		}
#endif
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void RadiusAttack1()
	{
#ifndef CLIENT_DLL
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

		switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetDamage(GUNKATA_MODE_B), 220, 360, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
		{
			case HIT_PLAYER:
			{
				if (m_iAnim2_pev_iuser4 % 2)
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/turbulent9_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
				else
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/turbulent9_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
				fDidHit = TRUE;
				break;
			}
			default:
			{
				break;
			}
		}
#endif
	}

	void RadiusAttack2()
	{
#ifndef CLIENT_DLL
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();

		struct
		{
			float RadiusDamageRadius = 220.0f;
			int RadiusDamageBitsDamageType = DMG_BULLET;
			bool RadiusDamageCanHeadshot = true;
			float RadiusDamageAmount = GetDamage(GUNKATA_MODE_B2);
		} Data;

		std::vector<CBaseEntity *> out;
		RadiusDamage(Data, vecSrc, m_pPlayer, m_pPlayer, std::back_inserter(out));
		for(CBaseEntity *ent : out)
		{
			CBasePlayer *player = dynamic_ent_cast<CBasePlayer *>(ent);
			if(player && player->m_bIsZombie)
			{
				ApplyKnockbackData(player, player->Center() - vecSrc, { 5000, 5000, 5000, 5000, 1.0f });
				player->pev->velocity.z = +200.0f;
			}
		}
#endif
	}

	bool HasSecondaryAttack() override { return true; }

	static void SwapAnimations(int iAnim1, int &a2, int &iAnim2) // void sub_101710E0(a1, a2, a3)
	{
		iAnim1 = iAnim2;
		iAnim2 -= 4;
		a2 = iAnim2;
	}

#ifndef CLIENT_DLL
	int m_iMode_pev_iuser1;
	EHANDLE m_pEntity_pev_waterlevel;
	int m_iStatus2_pev_watertype;
	int m_iAnim1_pev_iuser3;
	int m_iAnim2_pev_iuser4;
	time_point_t m_flNextSpecialAttack1_pev_fuser1;
	time_point_t m_flNextSpecialAttack2_pev_fuser2;
	time_point_t m_flNextSpecialAttack3_pev_fuser3;
	time_point_t m_flNextSpecialAttack4_pev_fuser4;
	time_point_t m_flNextSpecialAttack5_pev_teleport_time;
	time_point_t m_flNextSpecialAttack6_pev_air_finished;
	time_point_t m_flNextSpecialAttack7_pev_pain_finished;
	time_point_t m_flNextSpecialAttack8_pev_dmg_take;
	time_point_t m_flNextSpecialAttack9_pev_dmg_save;
	time_point_t m_flNextSpecialAttack10_pev_dmgtime;
	time_point_t m_flNextSpecialAttack11_pev_speed;
#endif

};
LINK_ENTITY_TO_CLASS(weapon_gunkata, CGunkata)

}
