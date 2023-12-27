/*
wpn_cannon.cpp - Black Dragon Cannon from CSO
Copyright (C) 2019 Moemod Hyakuya

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
#include "z4b_ak47_angelicbeast.h"
#include "weapons/KnifeAttack.h"

#ifndef CLIENT_DLL
#include "soundent.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


#ifndef CLIENT_DLL
    class CAK47_AngelicBeastGrenade : public CBaseEntity
    {
    public:
        float GetArmorRatioModifier() { return 1.5; }
        int GetWeaponsId(void) { return WEAPON_Z4B_AK47_ANGELICBEAST; }
        void Spawn() override
        {
            Precache();

            SetThink(&CAK47_AngelicBeastGrenade::FollowThink);
            SetTouch(&CAK47_AngelicBeastGrenade::OnTouch);
            SET_MODEL(this->edict(), "models/z4b/ef_mechanic_arcane.mdl");

            pev->rendermode = kRenderTransAdd;
            pev->renderfx = kRenderFxNone;
            pev->renderamt = 200;
            pev->framerate = 30;
            pev->scale = 1.0;
            pev->solid = SOLID_BBOX;
            pev->movetype = MOVETYPE_FLYMISSILE;
            pev->nextthink = gpGlobals->time + 0.1s;

            UTIL_SetOrigin(pev, pev->origin);
            UTIL_SetSize(pev, Vector(-1.0, -1.0, -1.0), Vector(1.0, 1.0, 1.0));
        }

        void Precache() override
        {
            PRECACHE_MODEL("models/z4b/ef_mechanic_arcane.mdl");
            m_iSprExplo = PRECACHE_MODEL("sprites/z4b/ef_fexplo.spr");
            PRECACHE_SOUND("CK_WPN/WEAPON/Mechanic/arcane_explosion.wav");
        }

        void EXPORT OnTouch(CBaseEntity* pOther)
        {
            Explode(pOther);
        }

        void Explode(CBaseEntity* pOther)
        {
            EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "CK_WPN/WEAPON/Mechanic/arcane_explosion.wav", 0.8, ATTN_NORM, 0, PITCH_NORM);

            MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
            WRITE_BYTE(TE_TEMPSPRITE);
            WRITE_COORD(pev->origin.x);
            WRITE_COORD(pev->origin.y);
            WRITE_COORD(pev->origin.z);
            WRITE_SHORT(m_iSprExplo);
            WRITE_BYTE(8);
            WRITE_BYTE(200);
            WRITE_BYTE(30);
            MESSAGE_END();

            CBaseEntity* pAttacker = CBaseEntity::Instance(pev->owner);
            CBasePlayer* pAttackePlayer = nullptr;
            if (pAttacker && pAttacker->IsPlayer())
                pAttackePlayer = static_cast<CBasePlayer*>(pAttacker);

            if (!pAttackePlayer)
                return;

            TraceResult tr;
            CBaseEntity* pEntity = NULL;

            Vector vecEnd;
            int bInWater = (UTIL_PointContents(pev->origin) == CONTENTS_WATER);
            float flRadius = 130.0;
            while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, flRadius)) != NULL)
            {
                if (pEntity->edict() == pev->owner)
                    continue;

                if (pEntity->IsDormant())
                    continue;

                if (pEntity->pev->takedamage == DAMAGE_NO)
                    continue;

                if (pEntity->Classify() == CLASS_NONE)
                    continue;

                if (g_pGameRules->PlayerRelationship(pAttackePlayer, pEntity) == GR_TEAMMATE)
                    continue;

                vecEnd = pEntity->BodyTarget(pev->origin);

                UTIL_TraceLine(pev->origin, vecEnd, ignore_monsters, pev->owner, &tr);

                if (tr.flFraction < 1.0 && tr.pHit != pEntity->edict())
                    continue;

                int falloff = RadiusDamageAmount / flRadius;

                float flCurrentDamage = max(0, RadiusDamageAmount - (pev->origin - pEntity->pev->origin).Length() * falloff);

                pEntity->TakeDamage(pev, pAttackePlayer->pev, flCurrentDamage, DMG_EXPLOSION);

                Vector vecDirection = (pEntity->pev->origin - pAttackePlayer->pev->origin).Normalize();
                UTIL_AddKickRateBoost(pEntity, vecDirection.Make2D(), 1.0, 1500.0, 500);
            }

            SetThink(nullptr);
            SetTouch(nullptr);
            UTIL_Remove(this);
        }

        void EXPORT FollowThink()
        {
            if (m_flRemoveTime < gpGlobals->time)
                Explode(NULL);

            pev->nextthink = gpGlobals->time + 0.1s;
        }

        void Init(Vector vecVelocity, float flExplodeDamage, int iTeam)
        {
            m_flRemoveTime = gpGlobals->time + 5.0s;
            std::tie(RadiusDamageAmount, m_iTeam) = std::make_tuple(flExplodeDamage, iTeam);
            pev->velocity = std::move(vecVelocity);
        }

        KnockbackData GetKnockBackData() override { return { 500.0f, 650.0f, 800.0f, 300.0f, 0.5f }; }

        time_point_t m_flRemoveTime;;
        float RadiusDamageAmount;
        int m_iSprExplo;

    };
    LINK_ENTITY_TO_CLASS(ak47_angelicbeast_grenade, CAK47_AngelicBeastGrenade)
#endif

LINK_ENTITY_TO_CLASS(z4b_ak47_angelicbeast, CAK47_AngelicBeast)

void CAK47_AngelicBeast::Spawn(void)
{
	Base::Spawn();
	m_iId = WEAPON_Z4B_AK47_ANGELICBEAST;
    m_iDefaultAmmo2 = 100;
    m_flTimeNextCharge = m_flTimeHostler = gpGlobals->time;
}

void CAK47_AngelicBeast::Precache(void)
{
	Base::Precache();
	
	m_usFire = PRECACHE_EVENT(1, "events/ak47_angelicbeast.sc");
}

float CAK47_AngelicBeast::GetDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 116;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 248;
#endif
	return 36;
}

void CAK47_AngelicBeast::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(MaxClip, RANDOM_LONG(0, 1) ? ANIM_RELOAD : ANIM_RELOAD_R1, DefaultReloadTime))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = AccuracyDefault;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}


BOOL CAK47_AngelicBeast::Deploy(void)
{
    if (m_flTimeHostler != invalid_time_point)
    {
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += (gpGlobals->time - m_flTimeHostler) / 0.1s;
    }
    if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 100;

    ResetRunIdle();

	m_bDelayFire = true;
	m_flAccuracy = AccuracyDefault;
	m_iShotsFired = 0;

    if (DefaultDeploy(V_Model, P_Model, RANDOM_LONG(0, 1) ? ANIM_DRAW : ANIM_DRAW_R1, "ak47", UseDecrement() != FALSE))
    {
        return TRUE;
    }
    return FALSE;
	
}

float CAK47_AngelicBeast::GetSecondaryAttackDamage() const
{
    float flDamage = 50;
    if (m_iKnifeAnim == ANIM_KNIFEATTACK2)
    {
        flDamage = 70;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 1000;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 1400;
#endif
    }
    else if (m_iKnifeAnim == ANIM_KNIFEATTACK3)
    {
        flDamage = 100;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 1800;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 2300;
#endif
    }
    else
    {
        flDamage = 50;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 600;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 1000;
#endif
    }

   

    return flDamage;
}

float CAK47_AngelicBeast::GetRadiusDamage() const
{
    float flDamage = 100;
#ifndef CLIENT_DLL
    if (g_pModRunning->DamageTrack() == DT_ZB)
        flDamage = 1600;
    else if (g_pModRunning->DamageTrack() == DT_ZBS)
        flDamage = 2000;
#endif
    return flDamage;
}




void CAK47_AngelicBeast::DelaySecondaryAttack()
{
#ifndef CLIENT_DLL
    switch (KnifeAttack5(GetSecondaryAttackDamage(), 90, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
    {
    case HIT_NONE:
    {
        break;
    }
    case HIT_PLAYER:
    {
        UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_Hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
        m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
        break;
    }
    case HIT_WALL:
    {
        UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_Stab.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
        break;
    }
    }
#endif
}


void CAK47_AngelicBeast::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
    {
        m_iKnifeAnim = 0;

        if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
        {
            if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
            {
                if (m_iWeaponState)
                {
                    SendWeaponAnim(ANIM_RUNB, UseDecrement() != FALSE);

                    m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.73s;
                }
                else
                {

                    SendWeaponAnim(ANIM_RUN, UseDecrement() != FALSE);

                    m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.83s;
                }
            }

            m_flNextIdle = UTIL_WeaponTimeBase();
        }
        else
        {
            if (m_flNextIdle <= UTIL_WeaponTimeBase())
            {
                if (m_iWeaponState)
                {
                    SendWeaponAnim(ANIM_IDLEB, UseDecrement() != FALSE);
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

void CAK47_AngelicBeast::PrimaryAttack(void)
{
    if (m_iWeaponState == 2)
    {
        ChangeMode();
        pev->iuser1 = 1;    //mark to shoot after reset anim;
        return;
    }
    Base::PrimaryAttack();
}

void CAK47_AngelicBeast::SecondaryAttack(void)
{
    if (m_iWeaponState == 2)
    {
        ChangeMode();
        pev->iuser1 = 2;    //mark to shoot after reset anim;
        return;
    }

#ifndef CLIENT_DLL
    m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif

    if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] >= 100)
    {
        SendWeaponAnim(ANIM_SHOOT_SKILL, UseDecrement() != FALSE);

        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] -= 100;

        EMIT_SOUND_DYN(this->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/Mechanic/Arcane_Fire.WAV", 0.8, ATTN_NORM, 0, PITCH_NORM);

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, 0.0, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
        Vector vecProjDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

        CAK47_AngelicBeastGrenade* pEnt = static_cast<CAK47_AngelicBeastGrenade*>(CBaseEntity::Create("ak47_angelicbeast_grenade", vecSrc, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev)));
        if (pEnt)
        {
            pEnt->Init(vecProjDir * 1000, GetRadiusDamage(), m_pPlayer->m_iTeam);
        }
#endif

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.87s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.87s;

        ResetRunIdle();


        return;
    }

    if (m_iKnifeAnim == ANIM_KNIFEATTACK)
    {
        m_iKnifeAnim = ANIM_KNIFEATTACK2;
        pev->nextthink = gpGlobals->time + 0.133s;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
       
    }
    else if (m_iKnifeAnim == ANIM_KNIFEATTACK2)
    {
        m_iKnifeAnim = ANIM_KNIFEATTACK3;
        pev->nextthink = gpGlobals->time + 0.133s;

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6s;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.33s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
    }
    else
    {
        m_iKnifeAnim = ANIM_KNIFEATTACK;
        pev->nextthink = gpGlobals->time + 0.3s; 

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8s;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.566s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
    }

    SendWeaponAnim(m_iKnifeAnim, UseDecrement() != FALSE);
    SetThink(&CAK47_AngelicBeast::DelaySecondaryAttack);

    ResetRunIdle();
}

void CAK47_AngelicBeast::ItemPostFrame()
{
    if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 100)
    {
        if (m_flTimeNextCharge <= gpGlobals->time)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
            m_flTimeNextCharge = gpGlobals->time + 0.1s;
        }
    }

    if (m_pPlayer->pev->button & IN_USE && !m_iWeaponState)
    {
        if (RANDOM_LONG(0, 1))
        {

            SendWeaponAnim(ANIM_INSEPCT, UseDecrement() != FALSE);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.8s;
        }
        else
        {

            SendWeaponAnim(ANIM_HANDSKILL, UseDecrement() != FALSE);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.3s;
        }


        ResetRunIdle();    
    }

    if (m_flNextPrimaryAttack <= 0.0s)
    {
        if (m_iWeaponState == 1)
        {
            m_iWeaponState = 2;
            m_pPlayer->ResetMaxSpeed();
        }
        else if (m_iWeaponState == 3)
        {
            m_iWeaponState = 0;
            m_pPlayer->ResetMaxSpeed();

            if (pev->iuser1 == 1)
            {
                PrimaryAttack();
                pev->iuser1 = 0;
            }
            else  if (pev->iuser1 == 2)
            {
                SecondaryAttack();
                pev->iuser1 = 0;
            }

        }
    }
    return CBasePlayerWeapon::ItemPostFrame();
}

void CAK47_AngelicBeast::ChangeMode()
{
    if (m_iWeaponState == 0)
    {
        SendWeaponAnim(ANIM_CHANGEMODE, UseDecrement() != FALSE);

        m_iWeaponState = 1;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
    }
    else if (m_iWeaponState == 1 || m_iWeaponState == 3)
    {
        //In changing return!
        return;
    }
    else if (m_iWeaponState == 2)
    {
        SendWeaponAnim(ANIM_CHANGEMODEB, UseDecrement() != FALSE);

        m_pPlayer->ResetMaxSpeed();
        m_iWeaponState = 3;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
    }

    ResetRunIdle();

}


}
