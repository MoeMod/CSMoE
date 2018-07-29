#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_shared/wpn_knife.h"
#include "knife_default.h"

enum knife_e
{
	KNIFE_IDLE,
	KNIFE_ATTACK1HIT,
	KNIFE_ATTACK2HIT,
	KNIFE_DRAW,
	KNIFE_STABHIT,
	KNIFE_STABMISS,
	KNIFE_MIDATTACK1HIT,
	KNIFE_MIDATTACK2HIT
};

BOOL CKnifeHelper_Default::Deploy(CKnife *pKnife)
{
	EMIT_SOUND(ENT(pKnife->m_pPlayer->pev), CHAN_ITEM, "weapons/knife_deploy1.wav", 0.3, 2.4);

	pKnife->m_fMaxSpeed = 250;
	pKnife->m_iSwing = 0;
	pKnife->m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	pKnife->m_pPlayer->m_bShieldDrawn = false;

	return pKnife->DefaultDeploy("models/v_knife.mdl", "models/p_knife.mdl", KNIFE_DRAW, "knife", UseDecrement(pKnife) != FALSE);
}

void CKnifeHelper_Default::Holster(CKnife *pKnife, int skiplocal)
{
	pKnife->m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
}

int CKnifeHelper_Default::Swing(CKnife *pKnife, int fFirst)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(pKnife->m_pPlayer->pev->v_angle);
	Vector vecSrc = pKnife->m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 48;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pKnife->m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(pKnife->m_pPlayer->pev), &tr);

		if (tr.flFraction < 1)
		{
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(pKnife->m_pPlayer->pev));

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction >= 1)
	{
		if (fFirst)
		{
			switch ((pKnife->m_iSwing++) % 2)
			{
			case 0: pKnife->SendWeaponAnim(KNIFE_MIDATTACK1HIT, UseDecrement(pKnife) != FALSE); break;
			case 1: pKnife->SendWeaponAnim(KNIFE_MIDATTACK2HIT, UseDecrement(pKnife) != FALSE); break;
			}

			pKnife->m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35;
			pKnife->m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;

			pKnife->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2;

			if (RANDOM_LONG(0, 1))
				EMIT_SOUND_DYN(ENT(pKnife->m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(pKnife->m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);

#ifndef CLIENT_DLL
			pKnife->m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		}
	}
	else
	{
		fDidHit = TRUE;

		switch ((pKnife->m_iSwing++) % 2)
		{
		case 0: pKnife->SendWeaponAnim(KNIFE_MIDATTACK1HIT, UseDecrement(pKnife) != FALSE); break;
		case 1: pKnife->SendWeaponAnim(KNIFE_MIDATTACK2HIT, UseDecrement(pKnife) != FALSE); break;
		}

		pKnife->m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4;
		pKnife->m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;

		pKnife->m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2;

		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
		pKnife->SetPlayerShieldAnim();

#ifndef CLIENT_DLL
		pKnife->m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		ClearMultiDamage();
		if (pEntity)
		{
			if (pKnife->m_flNextPrimaryAttack + 0.4 < UTIL_WeaponTimeBase())
				pEntity->TraceAttack(pKnife->m_pPlayer->pev, 20, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET);
			else
				pEntity->TraceAttack(pKnife->m_pPlayer->pev, 15, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET);
		}
		ApplyMultiDamage(pKnife->m_pPlayer->pev, pKnife->m_pPlayer->pev);

		float flVol = 1;
#ifndef CLIENT_DLL
		int fHitWorld = TRUE;
#endif
		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				switch (RANDOM_LONG(0, 3))
				{
				case 0: EMIT_SOUND(ENT(pKnife->m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit1.wav", VOL_NORM, ATTN_NORM); break;
				case 1: EMIT_SOUND(ENT(pKnife->m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit2.wav", VOL_NORM, ATTN_NORM); break;
				case 2: EMIT_SOUND(ENT(pKnife->m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit3.wav", VOL_NORM, ATTN_NORM); break;
				case 3: EMIT_SOUND(ENT(pKnife->m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit4.wav", VOL_NORM, ATTN_NORM); break;
				}

				pKnife->m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

				if (!pEntity->IsAlive())
					return TRUE;

				flVol = 0.1;
#ifndef CLIENT_DLL
				fHitWorld = FALSE;
#endif
			}
		}

#ifndef CLIENT_DLL
		if (fHitWorld)
		{
			TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			EMIT_SOUND_DYN(ENT(pKnife->m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
		}
#endif

		pKnife->m_trHit = tr;
		pKnife->m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;

		pKnife->SetThink(&CKnife::Smack);
		pKnife->pev->nextthink = UTIL_WeaponTimeBase() + 0.2;
		pKnife->SetPlayerShieldAnim();
	}

	return fDidHit;
}

int CKnifeHelper_Default::Stab(CKnife *pKnife, int fFirst)
{
	BOOL fDidHit = FALSE;
	UTIL_MakeVectors(pKnife->m_pPlayer->pev->v_angle);
	Vector vecSrc = pKnife->m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pKnife->m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(pKnife->m_pPlayer->pev), &tr);

		if (tr.flFraction < 1)
		{
			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);

			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, ENT(pKnife->m_pPlayer->pev));

			vecEnd = tr.vecEndPos;
		}
	}

	if (tr.flFraction >= 1)
	{
		if (fFirst)
		{
			pKnife->SendWeaponAnim(KNIFE_STABMISS, UseDecrement(pKnife) != FALSE);

			pKnife->m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1;
			pKnife->m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;

			if (RANDOM_LONG(0, 1))
				EMIT_SOUND_DYN(ENT(pKnife->m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			else
				EMIT_SOUND_DYN(ENT(pKnife->m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash2.wav", VOL_NORM, ATTN_NORM, 0, 94);
#ifndef CLIENT_DLL
			pKnife->m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		}
	}
	else
	{
		fDidHit = TRUE;

#ifndef CLIENT_DLL
		pKnife->SendWeaponAnim(KNIFE_STABHIT, UseDecrement(pKnife) != FALSE);
#endif
		pKnife->m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.1;
		pKnife->m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1;

		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

#ifndef CLIENT_DLL
		pKnife->m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		float flDamage = 65.0;

		if (pEntity && pEntity->IsPlayer())
		{
			Vector2D vec2LOS;
			Vector vecForward = gpGlobals->v_forward;

			UTIL_MakeVectors(pKnife->m_pPlayer->pev->angles);

			vec2LOS = vecForward.Make2D();
			vec2LOS = vec2LOS.Normalize();

			if (DotProduct(vec2LOS, gpGlobals->v_forward.Make2D()) > 0.8)
				flDamage *= 3.0;
		}

		UTIL_MakeVectors(pKnife->m_pPlayer->pev->v_angle);
		ClearMultiDamage();
		if (pEntity)
			pEntity->TraceAttack(pKnife->m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, DMG_NEVERGIB | DMG_BULLET);
		ApplyMultiDamage(pKnife->m_pPlayer->pev, pKnife->m_pPlayer->pev);

		float flVol = 1;
#ifndef CLIENT_DLL
		int fHitWorld = TRUE;
#endif
		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				EMIT_SOUND(ENT(pKnife->m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_stab.wav", VOL_NORM, ATTN_NORM);
				pKnife->m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;

				if (!pEntity->IsAlive())
					return TRUE;

				flVol = 0.1;
#ifndef CLIENT_DLL
				fHitWorld = FALSE;
#endif
			}
		}

#ifndef CLIENT_DLL
		if (fHitWorld)
		{
			TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			EMIT_SOUND_DYN(ENT(pKnife->m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
		}
#endif

		pKnife->m_trHit = tr;
		pKnife->m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;

		pKnife->SetThink(&CKnife::Smack);
		pKnife->pev->nextthink = UTIL_WeaponTimeBase() + 0.2;
	}

	return fDidHit;
}
