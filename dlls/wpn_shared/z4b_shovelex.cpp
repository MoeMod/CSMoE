/*
z4b_oppovivo.cpp
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#include "z4b_shovelex.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(z4b_shovelex, CKnifeShovelEX)


int CKnifeShovelEX::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ShovelEXAmmo";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 100;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iId = WEAPON_KNIFE;
	p->iFlags = 0;
	p->iWeight = KNIFE_WEIGHT;

	return 1;
}

int CKnifeShovelEX::AddToPlayer(CBasePlayer* pPlayer)
{

	return CBasePlayerWeapon::AddToPlayer(pPlayer);
}

void CKnifeShovelEX::Precache()
{
	for (int i = 0; i < TYPE_NUM; i++)
	{
		PRECACHE_MODEL(V_CUSTOM_MODEL[i]);
		PRECACHE_MODEL(P_CUSTOM_MODEL[i]);
	}

	Base::Precache();
}

void CKnifeShovelEX::ItemPostFrame()
{
	
	if (m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE)
	{
		if (m_flTimeCheckMode < gpGlobals->time && m_flTimeCheckMode != invalid_time_point)
		{
			duration_t flNextAttack = 0s;
			switch (m_iLastAnim)
			{
			case BALISONG_LUMINOUSEDGE_SLASH1: flNextAttack = 0.73s; break;
			case BALISONG_LUMINOUSEDGE_SLASH2: flNextAttack = 0.93s; break;
			case BALISONG_LUMINOUSEDGE_SLASH3: flNextAttack = 0.43s; break;
			case BALISONG_LUMINOUSEDGE_STAB: flNextAttack = 1.43s; break;
			case BALISONG_LUMINOUSEDGE_STAB_R1: flNextAttack = 1.26s; break;
			case BALISONG_LUMINOUSEDGE_STAB_CONTINUE: flNextAttack = 1.0s; break;
			case BALISONG_LUMINOUSEDGE_STAB_CONTINUE_R1: flNextAttack = 0.96s; break;
			}

			m_flTimeCheckMode = invalid_time_point;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flNextAttack;
		}

		if (m_iClip < 100)
		{
			if (m_flTimeNextCharge <= gpGlobals->time)
			{
				m_iClip++;
				m_flTimeNextCharge = gpGlobals->time + 0.125s;

				if (m_iClip == 100)
				{
					m_iModeState = 0;
					SendWeaponAnim(BALISONG_LUMINOUSEDGE_CHANGEMODE2, UseDecrement() != FALSE);
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				}
			}
		}
	}

	if (m_pPlayer->pev->button & IN_RELOAD)
	{
		if (m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE)
		{
			if (m_iModeState)
				return;

			if (m_iClip >= 100)
			{
				m_iClip = 0;
				m_iModeState = 1;
				SendWeaponAnim(BALISONG_LUMINOUSEDGE_CHAGEMODE, UseDecrement() != FALSE);
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.33s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.33s;
			

#ifndef CLIENT_DLL
				UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

				Vector vecSrc = m_pPlayer->GetGunPosition();
				Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0.0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

				Vector vecKnifeDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;
				Vector vecAngles = UTIL_VecToAngles(vecKnifeDir);

				CThrowingKnifeEntity* pKnife = CThrowingKnifeEntity::Create(0, vecSrc, vecAngles, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);

				if (pKnife)
				{
					float flSpeed;
					if (m_pPlayer->pev->waterlevel == 3)
						flSpeed = 1400.0;
					else
						flSpeed = 1600.0;

					pKnife->pev->velocity = vecKnifeDir * flSpeed;
					pKnife->pev->speed = flSpeed;
				}
#endif

				ResetRunIdle();

				return;
			}
		}
		

		m_iStoredChange.clear();

		m_iWeaponState = 0;
		m_pPlayer->pev->button &= ~IN_RELOAD;

#ifndef CLIENT_DLL
		ItemInfo II;
		GetItemInfo(&II);

		MESSAGE_BEGIN(MSG_ONE, gmsgWeaponList2, NULL, m_pPlayer->pev);
		WRITE_STRING(SHOVELEX_CLASSNAME[m_iWeaponState]);
		WRITE_SHORT(CBasePlayer::GetAmmoIndex(II.pszAmmo1));
		WRITE_BYTE(II.iMaxAmmo1);
		WRITE_SHORT(CBasePlayer::GetAmmoIndex(II.pszAmmo2));
		WRITE_BYTE(II.iMaxAmmo2);
		WRITE_BYTE(II.iSlot);
		WRITE_BYTE(II.iPosition);
		WRITE_SHORT(II.iId);
		WRITE_BYTE(II.iFlags);
		MESSAGE_END();
#endif
		Deploy();
		
		return;
	}

	if (m_pPlayer->pev->button & IN_USE && m_flNextPrimaryAttack < 0s && m_flNextSecondaryAttack < 0s && m_flNextInspect < gpGlobals->time)
	{
		if (m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE && !m_iModeState)
		{
			if (gpGlobals->time > m_flNextInspect)
			{
				if (RANDOM_LONG(0, 1))
				{
					SendWeaponAnim(BALISONG_LUMINOUSEDGE_OBSERVE, 0);

					ResetRunIdle();

					m_flNextInspect = gpGlobals->time + 5.5s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.5s;
				}
				else
				{
					SendWeaponAnim(BALISONG_LUMINOUSEDGE_OBSERVE_R1, 0);

					ResetRunIdle();

					m_flNextInspect = gpGlobals->time + 4.6s;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.6s;
				}

			}
		}

		m_pPlayer->pev->button &= ~IN_USE;
	}


	return CBasePlayerWeapon::ItemPostFrame();
}

void CKnifeShovelEX::ResetState()
{
	m_iModeState = 0;
	m_flTimeCheckMode = invalid_time_point;
	m_flTimeHostler = gpGlobals->time;
	m_flTimeNextCharge = invalid_time_point;
	m_iSwing = 0;
	m_flNextInspect = invalid_time_point;
	m_iLastAnim = 0;

	if (m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE)
		m_iClip = 100;
	else
		m_iClip = 0;
}

BOOL CKnifeShovelEX::Deploy()
{
//#ifndef CLIENT_DLL
//	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", "omoh_bl");
//	m_pPlayer->SetNewPlayerModel("models/player/omoh_bl/omoh_bl.mdl");
//#endif
	m_fMaxSpeed = 250;
	m_iSwing = 0;
	if (m_iWeaponState == TYPE_BCAXE_VIP)
	{
		m_iModeState = 0;
	}
	
	ResetRunIdle();

	int iAnim = ANIM_DRAW;

	switch (m_iWeaponState)
	{
	case TYPE_BTFKNIFE: iAnim = RANDOM_LONG(0, 1) ? BTFKNIFE_DRAW_R1 : BTFKNIFE_DRAW; break;
	case TYPE_AXE: iAnim = RANDOM_LONG(0, 1) ? ANIM_SKILL : ANIM_DRAW; break;
	case TYPE_BALISONG_LUMINOUSEDGE: 
		{
			if (m_flTimeHostler != invalid_time_point)
			{
				m_iClip += (gpGlobals->time - m_flTimeHostler) / 0.125s;
			}
			if (m_iClip >= 100)
			{
				m_iClip = 100;
				m_iModeState = 0;
			}
			
			if (m_iModeState)
			{
				iAnim = BALISONG_LUMINOUSEDGE_DRAW2;
			}
			else
			{
				iAnim = RANDOM_LONG(0, 1) ? BALISONG_LUMINOUSEDGE_DRAW : BALISONG_LUMINOUSEDGE_DRAW_R1;
			}

			
			break;
		}
	}

	if (DefaultDeploy(V_CUSTOM_MODEL[m_iWeaponState], P_CUSTOM_MODEL[m_iWeaponState], iAnim, AnimExtension, UseDecrement() != FALSE))
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		pev->body = m_pPlayer->m_bIsFemale ? 1 : 2;
		return TRUE;
	}
	return FALSE;
}

void CKnifeShovelEX::Spawn()
{
	m_iKnifeID = KNIFE_SHOVELEX;
	pev->classname = MAKE_STRING("z4b_shovelex");
	m_iId = WEAPON_KNIFE;
	Precache();
	SET_MODEL(ENT(pev), "models/w_knife.mdl");
	ResetState();
	FallInit();
}

void CKnifeShovelEX::MaterialSound(float flRadius)
{
#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecStart = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecStart + gpGlobals->v_forward * flRadius;
	TraceResult ptr;
	UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &ptr);

	const char* rgsz;

	switch (UTIL_TextureHit(&ptr, vecStart, vecEnd))
	{
	case CHAR_TEX_WOOD:
	{
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
		case TYPE_KNIFE:
		case TYPE_SHOVEL:
		case TYPE_KNUCKLE:
		case TYPE_BCAXE_VIP:
		case TYPE_BTFKNIFE:
			if (m_bSeondaryAttack)
				rgsz = "CK_WPN/WEAPON/AXE/AXE_WOOD_STAB.WAV";
			else
				rgsz = "CK_WPN/WEAPON/AXE/AXE_WOOD_SWING.WAV";
			break;
		case TYPE_BALISONG_LUMINOUSEDGE:
		{
			if (m_iModeState)
			{
				if (m_bSeondaryAttack)
					rgsz = "CK_WPN/WEAPON/Fist/Fist_Wood_Stab.WAV";
				else
					rgsz = "CK_WPN/WEAPON/Fist/Fist_Wood_Swing.WAV";
			}
			else
			{
				if (m_bSeondaryAttack)
					rgsz = "CK_WPN/WEAPON/DualKarambit/DualKarambit_Wood_Stab.WAV";
				else
					rgsz = "CK_WPN/WEAPON/DualKarambit/DualKarambit_Wood_Swing.WAV";
			}
			break;
		}	
		case TYPE_HAWLCHICKEN:
			if (m_bSeondaryAttack)
				rgsz = "CK_WPN/WEAPON/RubberChicken/RubberChicken_Wood_Stab.WAV";
			else
				rgsz = "CK_WPN/WEAPON/RubberChicken/RubberChicken_Wood_Swing.WAV";
			break;
		default:

			break;
		}

		break;
	}
	case CHAR_TEX_GRATE:
	case CHAR_TEX_COMPUTER:
	case CHAR_TEX_METAL:
	{
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
		case TYPE_KNIFE:
		case TYPE_AXE:
		case TYPE_SHOVEL:
		case TYPE_KNUCKLE:
		case TYPE_BCAXE_VIP:
		case TYPE_BTFKNIFE:
			if (m_bSeondaryAttack)
				rgsz = "CK_WPN/WEAPON/AXE/AXE_METAL_STAB.WAV";
			else
				rgsz = "CK_WPN/WEAPON/AXE/AXE_METAL_SWING.WAV";
			break;
		case TYPE_BALISONG_LUMINOUSEDGE:
		{
			if (m_iModeState)
			{
				if (m_bSeondaryAttack)
					rgsz = "CK_WPN/WEAPON/Fist/Fist_Metal_Stab.WAV";
				else
					rgsz = "CK_WPN/WEAPON/Fist/Fist_Metal_Swing.WAV";
			}
			else
			{
				if (m_bSeondaryAttack)
					rgsz = "CK_WPN/WEAPON/DualKarambit/DualKarambit_Metal_Stab.WAV";
				else
					rgsz = "CK_WPN/WEAPON/DualKarambit/DualKarambit_Metal_Swing.WAV";
			}
			break;
		}

		case TYPE_HAWLCHICKEN:
			if (m_bSeondaryAttack)
				rgsz = "CK_WPN/WEAPON/RubberChicken/RubberChicken_Metal_Stab.WAV";
			else
				rgsz = "CK_WPN/WEAPON/RubberChicken/RubberChicken_Metal_Swing.WAV";
			break;
		default:

			break;
		}

		break;
	}

	case CHAR_TEX_SNOW:
	{
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
		case TYPE_KNIFE:
		case TYPE_AXE:	
		case TYPE_SHOVEL:
		case TYPE_KNUCKLE:
		case TYPE_BCAXE_VIP:
		case TYPE_BTFKNIFE:
			if (m_bSeondaryAttack)
				rgsz = "CK_WPN/WEAPON/AXE/Axe_Snow_Stab.WAV";
			else
				rgsz = "CK_WPN/WEAPON/AXE/Axe_Snow_Swing.WAV";
			break;

		case TYPE_BALISONG_LUMINOUSEDGE:
		{
			if (m_iModeState)
			{
				if (m_bSeondaryAttack)
					rgsz = "CK_WPN/WEAPON/Fist/Fist_Snow_Stab.WAV";
				else
					rgsz = "CK_WPN/WEAPON/Fist/Fist_Snow_Swing.WAV";
			}
			else
			{
				if (m_bSeondaryAttack)
					rgsz = "CK_WPN/WEAPON/DualKarambit/DualKarambit_Snow_Stab.WAV";
				else
					rgsz = "CK_WPN/WEAPON/DualKarambit/DualKarambit_Snow_Swing.WAV";
			}
			break;
		}

		case TYPE_HAWLCHICKEN:
			if (m_bSeondaryAttack)
				rgsz = "CK_WPN/WEAPON/RubberChicken/RubberChicken_Snow_Stab.WAV";
			else
				rgsz = "CK_WPN/WEAPON/RubberChicken/RubberChicken_Snow_Swing.WAV";
			break;

		default:

			break;
		}

		break;
	}
	case CHAR_TEX_GLASS:
	{
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
		case TYPE_KNIFE:
		case TYPE_AXE:
		case TYPE_SHOVEL:
		case TYPE_KNUCKLE:
		case TYPE_BCAXE_VIP:
		case TYPE_BTFKNIFE:
			if (m_bSeondaryAttack)
				rgsz = "CK_WPN/WEAPON/AXE/Axe_BrokenGlass_Stab.WAV";
			else
				rgsz = "CK_WPN/WEAPON/AXE/Axe_BrokenGlass_Swing.WAV";
			break;

		case TYPE_BALISONG_LUMINOUSEDGE:
		{
			if (m_iModeState)
			{
				if (m_bSeondaryAttack)
					rgsz = "CK_WPN/WEAPON/Fist/Fist_BrokenGlass_Stab.WAV";
				else
					rgsz = "CK_WPN/WEAPON/Fist/Fist_BrokenGlass_Swing.WAV";
			}
			else
			{
				if (m_bSeondaryAttack)
					rgsz = "CK_WPN/WEAPON/DualKarambit/DualKarambit_BrokenGlass_Stab.WAV";
				else
					rgsz = "CK_WPN/WEAPON/DualKarambit/DualKarambit_BrokenGlass_Swing.WAV";
			}
			break;
		}

		case TYPE_HAWLCHICKEN:
			if (m_bSeondaryAttack)
				rgsz = "CK_WPN/WEAPON/RubberChicken/RubberChicken_BrokenGlass_Stab.WAV";
			else
				rgsz = "CK_WPN/WEAPON/RubberChicken/RubberChicken_BrokenGlass_Swing.WAV";
			break;

		default:

			break;
		}
	}

	default:
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
		case TYPE_KNIFE:
		case TYPE_AXE:
		case TYPE_SHOVEL:
		case TYPE_KNUCKLE:
		case TYPE_BCAXE_VIP:
		case TYPE_BTFKNIFE:
			if (m_bSeondaryAttack)
				rgsz = "CK_WPN/WEAPON/AXE/AXE_STONE_STAB.WAV";
			else
				rgsz = "CK_WPN/WEAPON/AXE/AXE_STONE_SWING.WAV";
			break;

		case TYPE_BALISONG_LUMINOUSEDGE:
		{
			if (m_iModeState)
			{
				if (m_bSeondaryAttack)
					rgsz = "CK_WPN/WEAPON/Fist/Fist_Stone_Stab.WAV";
				else
					rgsz = "CK_WPN/WEAPON/Fist/Fist_Stone_Swing.WAV";
			}
			else
			{
				if (m_bSeondaryAttack)
					rgsz = "CK_WPN/WEAPON/DualKarambit/DualKarambit_Stone_Stab.WAV";
				else
					rgsz = "CK_WPN/WEAPON/DualKarambit/DualKarambit_Stone_Swing.WAV";
			}
			break;
		}

		case TYPE_HAWLCHICKEN:
			if (m_bSeondaryAttack)
				rgsz = "CK_WPN/WEAPON/RubberChicken/RubberChicken_Stone_Stab.WAV";
			else
				rgsz = "CK_WPN/WEAPON/RubberChicken/RubberChicken_Stone_Swing.WAV";
			break;
		default:

			break;
		}

		break;
	}

	UTIL_EMIT_SOUND(ENT(0), CHAN_STATIC, rgsz, 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
#endif
}

void CKnifeShovelEX::ChangeWeapon()
{
	int iType = RANDOM_LONG(0, TYPE_NUM - 1);

	if(m_iStoredChange.size() >= TYPE_NUM)
	{
		m_iStoredChange.clear();
		m_iStoredChange.push_back(iType);
	}
	else
	{
		while (std::find(m_iStoredChange.begin(), m_iStoredChange.end(), iType) != m_iStoredChange.end())
		{
			iType = RANDOM_LONG(0, TYPE_NUM - 1);
		}

		m_iStoredChange.push_back(iType);
	}
	m_iWeaponState = iType;
#ifndef CLIENT_DLL
	ItemInfo II;
	GetItemInfo(&II);

	MESSAGE_BEGIN(MSG_ONE, gmsgWeaponList2, NULL, m_pPlayer->pev);
	WRITE_STRING(SHOVELEX_CLASSNAME[m_iWeaponState]);
	WRITE_SHORT(CBasePlayer::GetAmmoIndex(II.pszAmmo1));
	WRITE_BYTE(II.iMaxAmmo1);
	WRITE_SHORT(CBasePlayer::GetAmmoIndex(II.pszAmmo2));
	WRITE_BYTE(II.iMaxAmmo2);
	WRITE_BYTE(II.iSlot);
	WRITE_BYTE(II.iPosition);
	WRITE_SHORT(II.iId);
	WRITE_BYTE(II.iFlags);
	MESSAGE_END();
#endif

	ResetState();
}

void CKnifeShovelEX::Holster(int skiplocal)
{
	SetThink(nullptr);

	m_flTimeHostler = gpGlobals->time;

	return CBasePlayerWeapon::Holster(skiplocal);
}

void CKnifeShovelEX::PrimaryAttack(void)
{
	m_bSeondaryAttack = false;

	if (m_iWeaponState == TYPE_BCAXE_VIP)
	{
		if (m_iModeState)
		{
			OnAttackReleased();
			return;
		}
	}
	
	pev->iuser1 = 0;

	if (m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE)
	{
		int iAnim = BALISONG_LUMINOUSEDGE_SLASH1;

		if(m_iModeState == 1)
		{
			iAnim = m_iSwing ? BALISONG_LUMINOUSEDGE_SLASH_22 : BALISONG_LUMINOUSEDGE_SLASH_21;
		}
		else
		{
			switch (m_iSwing)
			{
			case 0:
				iAnim = BALISONG_LUMINOUSEDGE_SLASH1; break;
			case 1:
				iAnim = BALISONG_LUMINOUSEDGE_SLASH2; break;
			case 2:
				iAnim = BALISONG_LUMINOUSEDGE_SLASH3; break;
			}
		}


		SendWeaponAnim(iAnim, UseDecrement() != FALSE);
		m_iLastAnim = iAnim;
	}
	else
	{
		SendWeaponAnim(m_iSwing ? ANIM_SLASH2 : ANIM_SLASH1, UseDecrement() != FALSE);
	}
	
	

#ifndef CLIENT_DLL
	if (m_iWeaponState == TYPE_CLASSICKUKRI)
	{
		strcpy(m_pPlayer->m_szCustomSequence, "M-shoot-kukri");
		m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.6s;
	}
	else if (m_iWeaponState == TYPE_AXE)
	{
		strcpy(m_pPlayer->m_szCustomSequence, "M-shoot-Axe");
		m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.6s;
	}
	else
	{
		strcpy(m_pPlayer->m_szCustomSequence, "M-shoot-Keris");
		m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 0.6s;
	}

	m_pPlayer->pev->frame = 0;
	m_pPlayer->ResetSequenceInfo();
#endif

	if (m_iWeaponState == TYPE_AXE)
	{
		if (m_iSwing)
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/AXE/K_SHOOT_AXE_2.WAV", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
		else
			UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/AXE/K_SHOOT_AXE_1.WAV", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
	}
	
	if (m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE)
	{
		if (m_iModeState == 1)
		{
			m_iSwing++;

			m_iSwing %= 2;
		}
		else
		{
			m_flTimeCheckMode = gpGlobals->time + 0.336s;

			m_iSwing++;

			if (m_iSwing > 2)
				m_iSwing = 0;
		}
		
	}
	else
	{
		m_iSwing++;

		m_iSwing %= 2;
	}

	SetThink(&CKnifeShovelEX::DelayPrimaryAttack);
	pev->nextthink = gpGlobals->time + GetDelayPrimaryAttack();
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + GetNextPrimaryAttack();
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetPrimaryWeaponIdle();
	ResetRunIdle();
}

void CKnifeShovelEX::SecondaryAttack(void)
{
	if (m_iWeaponState == TYPE_BCAXE_VIP)
	{
		if (!m_iModeState)
		{
			m_flTimeCheckMode = gpGlobals->time + 0.743s;
			
			m_iModeState = 1;

			m_iSwing = 0;
			pev->iuser1 = 0;
			m_bSeondaryAttack = true;
#ifndef CLIENT_DLL
			strcpy(m_pPlayer->m_szCustomSequence, "M-shoot-knuckle");
			m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 1.2s;

			m_pPlayer->pev->frame = 0;
			m_pPlayer->ResetSequenceInfo();
#endif
			SendWeaponAnim(BCAXE_STAB2, UseDecrement() != FALSE);

			SetThink(&CKnifeShovelEX::DelaySecondaryAttack);
			pev->nextthink = gpGlobals->time + GetDelaySecondaryAttack();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + GetNextSecondaryAttack();
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.743s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetSecondaryWeaponIdle();
		}

		if (m_flTimeCheckMode < gpGlobals->time)
		{
			switch (m_iModeState)
			{
			case 1:
			{
				SendWeaponAnim(BCAXE_STABSTART);

				m_pPlayer->m_flNextAttack = 0.85s;
				m_flTimeCheckMode = gpGlobals->time + 0.85s;
				m_iModeState = 2;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.85s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.85s;
				break;
			}

			case 2:
			{
				SendWeaponAnim(BCAXE_STABLOOP);

				m_flTimeCheckMode = gpGlobals->time + 9999s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.3s;
				break;
			}
			}
		}
	}
	else
	{
		m_iSwing = 0;
		pev->iuser1 = 0;
		m_bSeondaryAttack = true;
#ifndef CLIENT_DLL
		if (m_iWeaponState == TYPE_SHOVELEX ||
			m_iWeaponState == TYPE_SHOVEL ||
			m_iWeaponState == TYPE_HAWLCHICKEN)
		{
			strcpy(m_pPlayer->m_szCustomSequence, "M-shoot-Shovel");
			m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 1.0s;
		}
		else
		{
			strcpy(m_pPlayer->m_szCustomSequence, "M-shoot-knuckle");
			m_pPlayer->m_flCustomSequenceUntilTime = gpGlobals->time + 1.2s;
		}

		m_pPlayer->pev->frame = 0;
		m_pPlayer->ResetSequenceInfo();
#endif

		if (m_iWeaponState == TYPE_BTFKNIFE)
		{
			int iAnim = RANDOM_LONG(0, 1) ? BTFKNIFE_STAB1 : BTFKNIFE_STAB_R1;

			SendWeaponAnim(iAnim, UseDecrement() != FALSE);		
		}
		else if (m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE)
		{
			int iAnim = BALISONG_LUMINOUSEDGE_STAB;
			if (m_iModeState)
				iAnim = BALISONG_LUMINOUSEDGE_STAB_2;
			else
			{
				if(m_iLastAnim == BALISONG_LUMINOUSEDGE_STAB || m_iLastAnim == BALISONG_LUMINOUSEDGE_STAB_R1)
					iAnim = RANDOM_LONG(0, 1) ? BALISONG_LUMINOUSEDGE_STAB_CONTINUE : BALISONG_LUMINOUSEDGE_STAB_CONTINUE_R1;
				else
					iAnim = RANDOM_LONG(0, 1) ? BALISONG_LUMINOUSEDGE_STAB : BALISONG_LUMINOUSEDGE_STAB_R1;
			}
				
			SendWeaponAnim(iAnim, UseDecrement() != FALSE);
			m_iLastAnim = iAnim;
			m_flTimeCheckMode = gpGlobals->time + 1.3s;
			//PLAY SOUND
		}
		else if (m_iWeaponState == TYPE_SHOVELEX)
		{
			SendWeaponAnim(RANDOM_LONG(0, 1) ? ANIM_STAB1 : ANIM_SKILL, UseDecrement() != FALSE);
			//PLAY SOUND
		}
		else
		{
			SendWeaponAnim(RANDOM_LONG(0, 1) ? ANIM_STAB1 : ANIM_STAB2, UseDecrement() != FALSE);
			//PLAY SOUND
		}

		SetThink(&CKnifeShovelEX::DelaySecondaryAttack);
		pev->nextthink = gpGlobals->time + GetDelaySecondaryAttack();

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + GetNextSecondaryAttack();
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetSecondaryWeaponIdle();

	}

	ResetRunIdle();
}

void CKnifeShovelEX::DelayPrimaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack5(GetPrimaryAttackDamage(), 85, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
		case HIT_PLAYER:
		{
			switch (m_iWeaponState)
			{
			case TYPE_SHOVELEX:
			case TYPE_KUKRI:
			case TYPE_CLASSICKUKRI:
			case TYPE_KNIFE:
			case TYPE_AXE:
			case TYPE_HAWLCHICKEN:
			case TYPE_SHOVEL:
			case TYPE_KNUCKLE:
			case TYPE_BCAXE_VIP:
			case TYPE_BTFKNIFE:
				UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_STATIC, "CK_WPN/WEAPON/AXE/K_SHOOT_AXE_PLAYER.WAV", 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
			case TYPE_BALISONG_LUMINOUSEDGE:
			{
				if (m_iModeState)
				{
					UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_STATIC, "CK_WPN/WEAPON/Fist/Fist_Hit.WAV", 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
	
				}
				else
				{
					UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_STATIC, "CK_WPN/WEAPON/DualKarambit/DualKarambit_Hit.WAV", 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
				}
				break;
			}
			default:

				break;
			}

			break;
		}
		case HIT_WALL:
		{
			MaterialSound(90);
			break;
		}
	}
#endif

	if (m_iWeaponState == TYPE_KNUCKLE ||
		m_iWeaponState == TYPE_BTFKNIFE ||
		m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE)
	{
		if (++pev->iuser1 & 1)
		{
			if (m_iWeaponState == TYPE_KNUCKLE)
			{
				pev->nextthink = gpGlobals->time + 0.23s;
			}
			else if (m_iWeaponState == TYPE_BTFKNIFE)
			{
				if (m_iSwing)
					pev->nextthink = gpGlobals->time + 0.23s;	//ANIM_SLASH1
				else
					pev->nextthink = gpGlobals->time + 0.29s;	//ANIM_SLASH2
			}
			else if (m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE)
			{
				if (m_iModeState)
				{
					if (m_iSwing)
						pev->nextthink = gpGlobals->time + 0.2s;	//ANIM_SLASH1
					else
						pev->nextthink = gpGlobals->time + 0.3s;	//ANIM_SLASH2
				}
				else
				{
					SetThink(nullptr);
				}
				
			}
					
			SetThink(&CKnifeShovelEX::DelayPrimaryAttack);
		}
		else
		{
			SetThink(nullptr);
		}
	}

	
}

void CKnifeShovelEX::DelaySecondaryAttack()
{
#ifndef CLIENT_DLL
	switch (KnifeAttack5(GetSecondaryAttackDamage(), 135, 120, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
	{
		case HIT_PLAYER:
		{
			switch (m_iWeaponState)
			{
			case TYPE_SHOVELEX:
			case TYPE_KUKRI:
			case TYPE_CLASSICKUKRI:
			case TYPE_KNIFE:
			case TYPE_AXE:
			case TYPE_HAWLCHICKEN:
			case TYPE_SHOVEL:
			case TYPE_KNUCKLE:
			case TYPE_BCAXE_VIP:
			case TYPE_BTFKNIFE:
				UTIL_EMIT_SOUND(ENT(0), CHAN_STATIC, "CK_WPN/WEAPON/AXE/K_SHOOT_AXE_PLAYER.WAV", 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
			case TYPE_BALISONG_LUMINOUSEDGE:
			{
				if (m_iModeState)
				{
					UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_STATIC, "CK_WPN/WEAPON/Fist/Fist_Hit.WAV", 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));

				}
				else
				{
					UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_STATIC, "CK_WPN/WEAPON/DualKarambit/DualKarambit_Hit.WAV", 0.9, ATTN_NORM, 0, 96 + RANDOM_LONG(0, 15));
				}
				break;
			}
			default:

				break;
			}

			break;
		}
		case HIT_WALL:
		{
			MaterialSound(135);
			break;
		}
	}
#endif

	SetThink(nullptr);
}

void CKnifeShovelEX::OnAttackReleased()
{
	if (m_iModeState == 1)
	{
		m_iModeState = 0;
		return;
	}

	SendWeaponAnim(BCAXE_STABSHOT);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;

	DelaySecondaryAttack();

	m_iModeState = 0;
}

void CKnifeShovelEX::WeaponIdle(void)
{
	if (m_iWeaponState == TYPE_BCAXE_VIP)
	{
		if (m_iModeState && !(m_pPlayer->pev->button & IN_ATTACK2))
		{
			OnAttackReleased();
		}
	}
	
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_iSwing = 0;

		if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
		{
			if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
			{
				if (m_iWeaponState == TYPE_BCAXE_VIP)
				{
					if (m_iModeState == 2)
						SendWeaponAnim(BCAXE_STABRUN, UseDecrement() != FALSE);
					else
						SendWeaponAnim(BCAXE_RUN, UseDecrement() != FALSE);
				}
				else if (m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE)
				{
					if (m_iModeState)
						SendWeaponAnim(BALISONG_LUMINOUSEDGE_RUN2, UseDecrement() != FALSE);
					else
						SendWeaponAnim(BALISONG_LUMINOUSEDGE_RUN, UseDecrement() != FALSE);
				}
				else
				{
					SendWeaponAnim(ANIM_RUN, UseDecrement() != FALSE);				
				}		

				m_flNextRunIdle = UTIL_WeaponTimeBase() + GetNextRunTime();
			}

			m_flNextIdle = UTIL_WeaponTimeBase();
		}
		else
		{
			if (m_flNextIdle <= UTIL_WeaponTimeBase())
			{
				if (m_iWeaponState == TYPE_BCAXE_VIP)
				{
					if (m_iModeState == 2)
						SendWeaponAnim(BCAXE_STABLOOP, UseDecrement() != FALSE);
					else
						SendWeaponAnim(BCAXE_IDLE1, UseDecrement() != FALSE);
				}
				else if (m_iWeaponState == TYPE_BALISONG_LUMINOUSEDGE)
				{
					if (m_iModeState)
						SendWeaponAnim(BALISONG_LUMINOUSEDGE_IDLE2, UseDecrement() != FALSE);
					else
						SendWeaponAnim(BALISONG_LUMINOUSEDGE_IDLE1, UseDecrement() != FALSE);
				}
				else
				{
					SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
				}
			
				m_flNextIdle = UTIL_WeaponTimeBase() + 20s;
			}

			m_flNextRunIdle = UTIL_WeaponTimeBase();
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
	}

}

}
