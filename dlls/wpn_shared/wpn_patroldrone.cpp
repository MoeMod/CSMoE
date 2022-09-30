#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_patroldrone.h"
#include "weapons/KnifeAttack.h"
#include "monsters.h"
#include "decals.h"
#include "gamerules.h"
#include "cbase/cbase_hash.h"

#ifndef CLIENT_DLL
#include "effects.h"
#include "customentity.h"
#include "player/player_mod_strategy.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_patroldrone, CPatrolDrone)

void CPatrolDrone::Spawn(void)
{
	Precache();
	m_iId = WEAPON_PATROLDRONE;
	SET_MODEL(ENT(pev), "models/w_patroldrone.mdl");

	m_iDefaultAmmo = PATROLDRONE_MAX_CLIP;

	for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
	{
		m_iClipAmmo[i] = PATROLDRONE_MAX_AMMO;
		m_flTimeHolster[i] = gpGlobals->time;
		m_flTimeNextCharge[i] = gpGlobals->time;
	}
	
	FallInit();
}

void CPatrolDrone::Precache(void)
{
	PRECACHE_MODEL("sprites/muzzleflash105.spr");

	PRECACHE_MODEL("models/v_patroldrone.mdl");
	PRECACHE_MODEL("models/p_patroldrone.mdl");
	PRECACHE_MODEL("models/w_patroldrone.mdl");

}

int CPatrolDrone::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "patroldrone";
	p->iMaxAmmo1 = PATROLDRONE_MAX_CLIP;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_PATROLDRONE;
	p->iWeight = HEGRENADE_WEIGHT;
	p->iFlags = 0;

	return 1;
}

BOOL CPatrolDrone::Deploy(void)
{
	m_fMaxSpeed = 260;
#ifndef CLIENT_DLL
	SetThink(nullptr);
	m_iState = 0;
	for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
	{
		if (!m_pPatrolDrone[i])
		{
			if (m_flTimeHolster[i] != invalid_time_point && m_iClipAmmo[i] != -1)
			{
				m_iClipAmmo[i] += (gpGlobals->time - m_flTimeHolster[i]) / PATROLDRONE_CHARGE_INTERVAL;
			}
		}
		if (m_iClipAmmo[i] >= PATROLDRONE_MAX_AMMO)
			m_iClipAmmo[i] = PATROLDRONE_MAX_AMMO;
	}

	MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
	WRITE_BYTE(23);
	WRITE_BYTE(PATROLDRONE_MAX_CLIP);
	for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
	{
		WRITE_SHORT(m_iClipAmmo[i]);
		if (m_pPatrolDrone[i])
		{
			if(m_pPatrolDrone[i]->GetState() == PATROLDRONE_ATTACK)
				WRITE_BYTE(PATROLDRONE_ATTACK);
			else
				WRITE_BYTE(PATROLDRONE_WANDER);
		}
		else
			WRITE_BYTE(PATROLDRONE_OFF);
	}
	MESSAGE_END();
#endif
	if (DefaultDeploy("models/v_patroldrone.mdl", "models/p_patroldrone.mdl", ANIM_DEPLOY, "dragontail", UseDecrement() != FALSE, 0.1s))
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75s;
		return TRUE;
	}
	return FALSE;
}

BOOL CPatrolDrone::CanHolster(void)
{
	return CBasePlayerWeapon::CanHolster();
}

void CPatrolDrone::Holster(int skiplocal)
{
	for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
	{
		m_flTimeHolster[i] = gpGlobals->time;
	}
	return CBasePlayerWeapon::Holster(skiplocal);
}

#ifndef CLIENT_DLL
void CPatrolDrone::CallPatrol()
{
	static int iOrderCall, iOrder;
	Vector vecSrc = m_pPlayer->pev->origin;
	for (iOrderCall; iOrderCall < PATROLDRONE_MAX_CLIP; iOrderCall++)
	{
		if (!HasClip(iOrderCall))
			continue;

		if (!m_pPatrolDrone[iOrderCall])
		{
			
			m_pPatrolDrone[iOrderCall] = CPatrolDroneGrenade::CreatePatrolDrone(m_pPlayer, this, vecSrc, g_vecZero, m_pPlayer->m_iTeam, iOrderCall, iOrder);
			iOrder++;
			SetThink(&CPatrolDrone::CallPatrol);
			pev->nextthink = gpGlobals->time + 0.15s;
			break;
		}
		else
			m_pPatrolDrone[iOrderCall]->SetState(PATROLDRONE_WANDER);
	}

	if (iOrderCall >= PATROLDRONE_MAX_CLIP)
	{
		iOrder = 0;
		iOrderCall = 0;
		SetThink(nullptr);
		pev->nextthink = invalid_time_point;
	}
}

void CPatrolDrone::CallReturn()
{
	static int iOrderBack;
	for (iOrderBack ; iOrderBack < PATROLDRONE_MAX_CLIP; iOrderBack++)
	{
		if (m_pPatrolDrone[iOrderBack])
		{
			m_pPatrolDrone[iOrderBack]->ReturnBack();
			SetThink(&CPatrolDrone::CallReturn);
			pev->nextthink = gpGlobals->time + 0.15s;
			break;
		}
		else
		{
			continue;
		}
	}

	if (iOrderBack >= PATROLDRONE_MAX_CLIP)
	{
		iOrderBack = 0;
		SetThink(nullptr);
		pev->nextthink = invalid_time_point;
	}
}
#endif

void CPatrolDrone::ItemPostFrame()
{
#ifndef CLIENT_DLL
	for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
	{
		if (!m_pPatrolDrone[i])
		{
			if (m_iClipAmmo[i] != -1 && m_iClipAmmo[i] < PATROLDRONE_MAX_AMMO)
			{
				if (m_flTimeNextCharge[i] != invalid_time_point && m_flTimeNextCharge[i] <= gpGlobals->time)
				{
					m_iClipAmmo[i] += 1;
					m_flTimeNextCharge[i] = gpGlobals->time + PATROLDRONE_CHARGE_INTERVAL;
				}
			}
			
		}
	}
#endif
	return CBasePlayerWeapon::ItemPostFrame();
}

#ifndef CLIENT_DLL
bool CPatrolDrone::HasPatrolDrone()
{
	bool alloutside = true;
	for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
	{
		if (!HasClip(i))
			continue;

		if (!m_pPatrolDrone[i])
			alloutside = false;
	}

	return alloutside;
}

bool CPatrolDrone::HasClip()
{
	for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
	{
		if (float(m_iClipAmmo[i]) / float(PATROLDRONE_MAX_AMMO) > 0.3)
			return true;
	}
	return false;
}

bool CPatrolDrone::HasClip(int iSlot)
{
	if (float(m_iClipAmmo[iSlot]) / float(PATROLDRONE_MAX_AMMO) > 0.3)
		return true;
	return false;
}

bool CPatrolDrone::HasDroneWithClip()
{
	for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
	{
		if (m_pPatrolDrone[i])
			continue;

		if (float(m_iClipAmmo[i]) / float(PATROLDRONE_MAX_AMMO) > 0.3)
			return true;
	}
	return false;
}

bool CPatrolDrone::HasDroneOutside()
{
	for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
	{
		if (m_pPatrolDrone[i])
			return true;
	}
	return false;
}

#endif

void CPatrolDrone::PrimaryAttack(void)
{
#ifndef CLIENT_DLL
	m_iState = 0;
	if (HasDroneWithClip())
		m_iState = 1;
	else if (HasDroneOutside())
		m_iState = 2;

	if (m_iState == 1)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SendWeaponAnim(ANIM_PATROL, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		SetThink(&CPatrolDrone::CallPatrol);
		pev->nextthink = gpGlobals->time + 0.15s;
	}
	else if (m_iState == 2)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SendWeaponAnim(ANIM_RETURN, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		SetThink(&CPatrolDrone::CallReturn);
		pev->nextthink = gpGlobals->time + 0.15s;
	}
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8s;
	m_flTimeWeaponIdle2 = 0s;
	return;

	if (!HasPatrolDrone())
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SendWeaponAnim(ANIM_PATROL, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		SetThink(&CPatrolDrone::CallPatrol);
		pev->nextthink = gpGlobals->time + 0.1s;
	}
	else
	{

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SendWeaponAnim(ANIM_RETURN, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		SetThink(&CPatrolDrone::CallReturn);
		pev->nextthink = gpGlobals->time + 0.1s;
	}
#endif

}

void CPatrolDrone::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	for (int i = 0; i < PATROLDRONE_MAX_CLIP; i++)
	{
		if (m_pPatrolDrone[i])
		{
			if (m_pPatrolDrone[i]->GetState() == PATROLDRONE_ATTACK)
			{
				SendWeaponAnim(ANIM_THROW, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;
				m_pPatrolDrone[i]->Explode();
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
				break;
			}
		}
	}
	

#endif

}

void CPatrolDrone::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flTimeWeaponIdle2 < 10s)
	{
		m_flTimeWeaponIdle2 += m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.8s;
		SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 8.4s;
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
		m_flTimeWeaponIdle2 = 0s;
	}
}

#ifndef CLIENT_DLL
class CBeam;

LINK_ENTITY_TO_CLASS(grenade_patroldrone, CPatrolDroneGrenade);

void CPatrolDroneGrenade::Precache(void)
{
	PRECACHE_MODEL("models/w_patroldrone.mdl");

	m_iLightningIndex = PRECACHE_MODEL("sprites/ef_droneattacklightning.spr");
	m_iExpIndex = PRECACHE_MODEL("sprites/ef_droneexplosion.spr");

	PRECACHE_SOUND("weapons/patroldrone_attack_loop.wav");
	/*PRECACHE_SOUND("weapons/patroldrone_drone_fly1.wav");
	PRECACHE_SOUND("weapons/patroldrone_drone_fly2.wav");
	PRECACHE_SOUND("weapons/patroldrone_drone_fly3.wav");
	PRECACHE_SOUND("weapons/patroldrone_power_off.wav");
	PRECACHE_SOUND("weapons/patroldrone_power_on.wav");*/
	PRECACHE_SOUND("weapons/patroldrone_throw_exp1.wav");
	PRECACHE_SOUND("weapons/patroldrone_throw_exp2.wav");
}

void CPatrolDroneGrenade::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;

	if (pev->classname)
	{
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);
	}

	MAKE_STRING_CLASS("grenade", pev);
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	SET_MODEL(ENT(pev), "models/w_patroldrone.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pBeam = CBeam::BeamCreate("sprites/ef_droneattacklightning.spr", 160);
	pBeam->SetColor(255, 255, 255);
	pBeam->SetScrollRate(30);
	pBeam->SetBrightness(0);
	pBeam->SetNoise(30);
	pBeam->pev->effects |= EF_NODRAW;
}

void CPatrolDroneGrenade::ClearBeam()
{
	pBeam->SetBrightness(0);
	pBeam->pev->effects |= EF_NODRAW;
}

CPatrolDroneGrenade* CPatrolDroneGrenade::CreatePatrolDrone(CBasePlayer* pOwner, CPatrolDrone* pWeapon, Vector vecStart, Vector vecVelocity, int iTeam, int iSlot, int iOrder)
{
	CPatrolDroneGrenade* pPatrolDrone = CreateClassPtr<CPatrolDroneGrenade>();
	pPatrolDrone->Spawn();
	UTIL_SetOrigin(pPatrolDrone->pev, vecStart);
	pPatrolDrone->m_pOwner = pOwner;
	pPatrolDrone->m_pWeapon = pWeapon;
	pPatrolDrone->m_iSlot = iSlot;
	pPatrolDrone->m_iOrder = iOrder;
	pPatrolDrone->pev->velocity = vecVelocity;
	pPatrolDrone->pev->angles = UTIL_VecToAngles(-pPatrolDrone->pev->velocity);
	pPatrolDrone->pev->owner = ENT(pOwner->pev);
	pPatrolDrone->pev->sequence = 0;
	pPatrolDrone->pev->frame = 0;
	pPatrolDrone->ResetSequenceInfo();
	pPatrolDrone->m_fSequenceLoops = TRUE;
	pPatrolDrone->pev->framerate = 1.0f;
	pPatrolDrone->pev->nextthink = gpGlobals->time + 0.1s;
	pPatrolDrone->m_iTeam = iTeam;
	pPatrolDrone->m_iState = PATROLDRONE_WANDER;
	pPatrolDrone->m_flWanderThink = gpGlobals->time + 0.1s;
	pPatrolDrone->m_flAttackThink = gpGlobals->time + 0.1s;
	pPatrolDrone->SetThink(&CPatrolDroneGrenade::OnThink);

	return pPatrolDrone;
}

void CPatrolDroneGrenade::OnThink()
{
	if (!m_pOwner)
	{
		UTIL_Remove(pBeam);
		UTIL_Remove(this);
		return;
	}

	if (m_pOwner->m_bIsZombie || !m_pOwner->IsAlive())
	{
		UTIL_Remove(pBeam);
		UTIL_Remove(this);
		return;
	}

	CBasePlayerItem* pWeapon = m_pOwner->m_rgpPlayerItems[GRENADE_SLOT];

	if (!pWeapon)
	{
		UTIL_Remove(pBeam);
		UTIL_Remove(this);
		return;
	}

	while (pWeapon)
	{
		if (pWeapon->m_iId != WEAPON_PATROLDRONE)
		{
			UTIL_Remove(pBeam);
			UTIL_Remove(this);
			return;
		}

		pWeapon = pWeapon->m_pNext;
	}

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.01s;

	if (m_iState != m_iStateStored)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pOwner->pev);
		WRITE_BYTE(24);
		WRITE_BYTE(m_iSlot);
		WRITE_SHORT(m_pWeapon->m_iClipAmmo[m_iSlot]);
		WRITE_BYTE(m_iState);
		MESSAGE_END();

		m_iStateStored = m_iState;
	}

	if (m_flChargeThink < gpGlobals->time)
	{
		m_pWeapon->m_iClipAmmo[m_iSlot]--;
		if (m_pWeapon->m_iClipAmmo[m_iSlot] <= 0)
		{
			m_pWeapon->m_iClipAmmo[m_iSlot] = 0;
			m_iState = PATROLDRONE_RETURN;
		}

		m_flChargeThink = gpGlobals->time + PATROLDRONE_CHARGE_INTERVAL;
	}

	if (m_flAnimThink < gpGlobals->time)
	{
		ChangeAnim();
		m_flAnimThink = gpGlobals->time + 0.35s;
	}

	if (m_iState == PATROLDRONE_RETURN)
	{
		m_iStateStored = PATROLDRONE_RETURN;
		Vector vecOrigin = m_pOwner->pev->origin;
		Vector vecEntOrigin = pev->origin;
		Vector vecVelocity = (vecOrigin - vecEntOrigin).Normalize() * PATROLDRONE_MOVESPEED;
		pev->velocity = vecVelocity;

		float flDis = (pev->origin - m_pOwner->pev->origin).Length();
		if (flDis < 5.0f)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pOwner->pev);
			WRITE_BYTE(24);
			WRITE_BYTE(m_iSlot);
			WRITE_SHORT(m_pWeapon->m_iClipAmmo[m_iSlot]);
			WRITE_BYTE(PATROLDRONE_OFF);
			MESSAGE_END();

			UTIL_Remove(pBeam);
			UTIL_Remove(this);
		}
			
		return;
	}

	if (FindTargets())
	{
		m_iState = PATROLDRONE_ATTACK;

		pBeam->SetType(BEAM_ENTS);
		pBeam->SetStartEntity(this->entindex());
		pBeam->SetEndEntity(m_pTarget->entindex());
		pBeam->SetStartAttachment(0);
		pBeam->SetEndAttachment(0);
		pBeam->RelinkBeam();
		pBeam->SetBrightness(230);
		pBeam->pev->effects &= ~EF_NODRAW;

		if (vecLockOffset.IsZero())
		{
			vecLockOffset.x = RANDOM_LONG(-20, 20);
			vecLockOffset.y = RANDOM_LONG(-20, 20);
			vecLockOffset.z = RANDOM_LONG(0, 30);

			m_flAttackThink = gpGlobals->time;
		}

	}
	else
	{
		if (m_iState != PATROLDRONE_RETURN)
			m_iState = PATROLDRONE_WANDER;
	
		m_vecTarget.clear();
		ClearBeam();
		vecLockOffset = g_vecZero;
	}

	if (m_iState == PATROLDRONE_ATTACK)
	{
		vecEndOrigin = m_pTarget->pev->origin + vecLockOffset;

		if ((vecEndOrigin - pev->origin).Length() < 20)
		{
			UTIL_SetOrigin(pev, m_pTarget->pev->origin + vecLockOffset);
			pev->velocity = g_vecZero;
		}
		else
			pev->velocity = CalcVelocity(pev->origin, vecEndOrigin, PATROLDRONE_MOVESPEED);

		if (m_flAttackThink != invalid_time_point && m_flAttackThink < gpGlobals->time)
		{
			if (gpGlobals->time >= m_flNextSoundTime)
			{
				m_flNextSoundTime = gpGlobals->time + 0.9s;

				EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "weapons/patroldrone_attack_loop.wav", VOL_NORM, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 15));
			}
			Attack();
			m_flAttackThink = gpGlobals->time + PATROLDRONE_ATTACK_INTERVAL;
		}
	}
	else if (m_iState == PATROLDRONE_WANDER)
	{	
		vecEndOrigin = m_pOwner->pev->origin + vecOffset;
		//if (m_flMoveThink < gpGlobals->time)
		{
			m_flMoveThink = gpGlobals->time + 0.01s;
			
			if ((vecEndOrigin - pev->origin).Length() < 20)
			{
				UTIL_SetOrigin(pev, m_pOwner->pev->origin + vecOffset);
				pev->velocity = g_vecZero;
			}
			else
				pev->velocity = CalcVelocity(pev->origin, vecEndOrigin, PATROLDRONE_MOVESPEED);
		}

		if (m_flWanderThink != invalid_time_point && m_flWanderThink < gpGlobals->time)
		{
			vecOffset.x = RANDOM_LONG(-45, 45);
			vecOffset.y = RANDOM_LONG(-45, 45);
			vecOffset.z = RANDOM_LONG(0, 30);

			m_flWanderThink = gpGlobals->time + RANDOM_FLOAT(0.8, 1.5) * 1s;
		}	
	}

}

Vector CPatrolDroneGrenade::CalcVelocity(Vector vecSrc, Vector vecEnd, float flSpeed)
{
	Vector vecVelocity = (vecEnd - vecSrc).Normalize();
	return vecVelocity * flSpeed;
}

void CPatrolDroneGrenade::ChangeAnim()
{
	if (m_iState == PATROLDRONE_ATTACK)
	{
		if (pev->sequence == ANIM_IDLE_A || pev->sequence == ANIM_CHANGEA)
			pev->sequence = ANIM_CHANGEB;
		else if (pev->sequence == ANIM_CHANGEB)
			pev->sequence = ANIM_IDLE_B;
	}
	else
	{
		if (pev->sequence == ANIM_IDLE_B || pev->sequence == ANIM_CHANGEB)
			pev->sequence = ANIM_CHANGEA;
		else if (pev->sequence == ANIM_CHANGEA)
			pev->sequence = ANIM_IDLE_A;
	}
}

void CPatrolDroneGrenade::ReturnBack()
{
	m_flAttackThink = invalid_time_point;
	m_iState = PATROLDRONE_RETURN;
	Vector vecOrigin = m_pOwner->pev->origin;
	Vector vecEntOrigin = pev->origin;
	Vector vecVelocity = (vecOrigin - vecEntOrigin) * 10;
	pev->velocity = vecVelocity;
}

void CPatrolDroneGrenade::Explode()
{
	CBaseEntity* pEntity = NULL;
	Vector vecSrc = pev->origin;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, PATROLDRONE_EXPLODE_RADIUS)) != NULL)
	{
		if (pEntity->IsDormant())
			continue;

		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (pEntity == m_pOwner)
			continue;

		if (bIsWallBetweenPointEntity(vecSrc, pEntity))
			continue;

		pEntity->m_LastHitGroup = HITGROUP_CHEST;
		pEntity->TakeDamage(pev, m_pOwner->pev, GetExpDamage(), DMG_GENERIC | DMG_BULLET | DMG_NEVERGIB);

		Vector vecDirection = (pEntity->pev->origin - m_pOwner->pev->origin).Normalize();
		UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 3500.0, 0);
	}

	{
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_TEMPSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(m_iExpIndex);
		WRITE_BYTE(10);
		WRITE_BYTE(255);
		WRITE_BYTE(45);
		MESSAGE_END();
	}

	if (RANDOM_LONG(0, 1))
		EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "weapons/patroldrone_throw_exp1.wav", VOL_NORM, 0.25, 0, PITCH_NORM);
	else
		EMIT_SOUND_DYN(ENT(pev), CHAN_AUTO, "weapons/patroldrone_throw_exp2.wav", VOL_NORM, 0.25, 0, PITCH_NORM);

	m_pWeapon->m_iClipAmmo[m_iSlot] = -1;

	MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pOwner->pev);
	WRITE_BYTE(24);
	WRITE_BYTE(m_iSlot);
	WRITE_SHORT(m_pWeapon->m_iClipAmmo[m_iSlot]);
	WRITE_BYTE(PATROLDRONE_OFF);
	MESSAGE_END();

	SetThink(NULL);

	UTIL_Remove(pBeam);
	UTIL_Remove(this);


}

void CPatrolDroneGrenade::Attack()
{
	if (!m_pTarget)
		return;

	TraceResult tr;
	Vector vecSrc = m_pOwner->pev->origin;
	UTIL_TraceLine(vecSrc, m_pTarget->pev->origin, dont_ignore_monsters, m_pOwner->edict(), &tr);

	ClearMultiDamage();

	tr.iHitgroup = HITGROUP_CHEST;
	m_pTarget->TraceAttack(m_pOwner->pev, GetDirctDamage(), (tr.vecEndPos - vecSrc).Normalize(), &tr, DMG_NEVERGIB | DMG_BULLET);

	ApplyMultiDamage(pev, m_pOwner->pev);

	if (m_pTarget->IsPlayer())
	{
		CBasePlayer* player = (CBasePlayer*)m_pTarget;
		if (player->m_bIsZombie && !(player->m_iBuff & BUFF_IGNORE_KNOCKBACK))
		{

			auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(player->m_pModStrategy.get());
			if (strategy)
			{
				strategy->SetStunSpeedTime(1.0s, 70.0f);
				strategy->SetStunGravityTime(1.0s);
			}
		}
	}
}

bool CPatrolDroneGrenade::CheckTarget()
{
	if (m_pTarget)
	{
		if (!m_pTarget->IsAlive())
		{
			m_pTarget = NULL;
			return false;
		}
			
		float flDis = (m_pTarget->pev->origin - m_pOwner->pev->origin).Length();

		if (flDis > PATROLDRONE_DETECT_RADIUS)
		{
			m_pTarget = NULL;
			return false;
		}
			
		if (bIsWallBetweenEntity(m_pTarget, m_pOwner))
		{
			m_pTarget = NULL;
			return false;
		}

		return true;
	}
	return false;
}

bool CPatrolDroneGrenade::CheckOtherTarget()
{
	if (m_vecTarget.empty())
		return false;

	std::sort(m_vecTarget.begin(), m_vecTarget.end(), [](const std::pair<float, CBaseEntity*>& a, const std::pair<float, CBaseEntity*>& b) {return a.first < b.first; });

	if (m_vecTarget.size() >= m_iOrder + 1)
	{
		m_pTarget = m_vecTarget.at(m_iOrder).second;
		return true;
	}
	else
	{
		/*int id = ceil(double(m_iOrder) / 2);
		if (id >= m_vecTarget.size() - 1)
			id = 0;
		m_pTarget = m_vecTarget.at(id).second;*/
		if(m_vecTarget.size() == 1)
			m_pTarget = m_vecTarget.at(0).second;
		else
			m_pTarget = m_vecTarget.at(m_iOrder - m_vecTarget.size()).second;
		return true;
	}

	return false;
}

bool CPatrolDroneGrenade::FindTargets()
{
	bool find = false;
	CBaseEntity* pEntity = NULL;
	//CheckTarget();
	m_vecTarget.clear();
	m_pTarget = nullptr;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, m_pOwner->pev->origin, PATROLDRONE_DETECT_RADIUS)) != nullptr)
	{
		if (CheckTargetAvailable(pEntity))
		{
			float dis = (pEntity->pev->origin - m_pOwner->pev->origin).Length();
			
			m_vecTarget.push_back(std::make_pair(dis, pEntity));
			//m_pTarget = pEntity;
			//find = true;
		}
	}

	find = CheckOtherTarget();

	return find;
}


bool CPatrolDroneGrenade::bIsWallBetweenPointEntity(Vector vecSrc, CBaseEntity* pEntity)
{
	TraceResult tr;
	Vector vecEnd = pEntity->pev->origin;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, this->edict(), &tr);

	if (tr.flFraction < 1.0)
	{
		if (!tr.pHit)
			return false;

		CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);

		if (pHit)
		{
			if (pHit == pEntity)
				return false;

			if (!Q_strcmp(STRING(pHit->pev->classname), "holysword_parray") || !Q_strcmp(STRING(pHit->pev->classname), "y22s1holyswordmb_parray"))
				return false;
		}
		return true;
	}
	return false;
}

bool CPatrolDroneGrenade::CheckTargetAvailable(CBaseEntity* pEntity)
{
	if (!pEntity->IsPlayer() && pEntity->Classify() != CLASS_PLAYER_ALLY)
		return false;

	if (!pEntity->IsAlive())
		return false;

	if (g_pGameRules->PlayerRelationship(m_pOwner, pEntity) == GR_TEAMMATE)
		return false;

	if (pEntity->pev == m_pOwner->pev)
		return false;

	Vector vecSrc = m_pOwner->GetGunPosition();
	Vector vecDelta = (pEntity->Center() - vecSrc).Normalize();

	TraceResult tr;
	UTIL_TraceLine(m_pOwner->pev->origin, pEntity->pev->origin, missile, m_pOwner->edict(), &tr);

	if (tr.flFraction < 1.0)
	{
		if (!tr.pHit)
			return false;

		if (tr.pHit != pEntity->edict())
			return false;
	}

	return true;
}

float CPatrolDroneGrenade::GetDirctDamage()
{
	float flDamage = 2;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 36;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 92;
	return flDamage;
}

float CPatrolDroneGrenade::GetExpDamage()
{
	float flDamage = 40;
	if (g_pModRunning->DamageTrack() == DT_ZB)
	{
		if (enhancedzombie.value)
			flDamage = 14300;
		else
			flDamage = 9000;
	}		
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 2200;
	return flDamage;
}


#endif
}
